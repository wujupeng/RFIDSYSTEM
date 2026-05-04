#include "system_recommendations_page.h"
#include "../../network/grpc_client.h"
#include <QTableWidget>
#include <QHeaderView>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTimer>
#include <QDebug>
#include <QDateTime>

SystemRecommendationsPage::SystemRecommendationsPage(GrpcClient* grpcClient, QWidget* parent)
    : QWidget(parent)
    , table_(nullptr)
    , totalLabel_(nullptr)
    , highPriorityLabel_(nullptr)
    , mediumPriorityLabel_(nullptr)
    , lowPriorityLabel_(nullptr)
    , adoptionRateLabel_(nullptr)
    , refreshTimer_(nullptr)
    , grpcClient_(grpcClient)
    , cachedAdoptionRate_(0.0)
{
    current_user_ = "operator";

    setupUI();

    refreshTimer_ = new QTimer(this);
    connect(refreshTimer_, &QTimer::timeout, this, &SystemRecommendationsPage::refreshRecommendations);
    refreshTimer_->start(5000);

    refreshRecommendations();
}

void SystemRecommendationsPage::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    QLabel* titleLabel = new QLabel("系统建议 (System Recommendations)", this);
    titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; padding: 10px;");
    mainLayout->addWidget(titleLabel);

    QHBoxLayout* statsLayout = new QHBoxLayout();

    totalLabel_ = new QLabel("总建议数: 0", this);
    highPriorityLabel_ = new QLabel("高优先级: 0", this);
    highPriorityLabel_->setStyleSheet("color: #d32f2f; font-weight: bold;");
    mediumPriorityLabel_ = new QLabel("中优先级: 0", this);
    mediumPriorityLabel_->setStyleSheet("color: #f57c00; font-weight: bold;");
    lowPriorityLabel_ = new QLabel("低优先级: 0", this);
    lowPriorityLabel_->setStyleSheet("color: #388e3c;");
    adoptionRateLabel_ = new QLabel("采纳率: --", this);
    adoptionRateLabel_->setStyleSheet("color: #1976d2; font-weight: bold;");

    statsLayout->addWidget(totalLabel_);
    statsLayout->addWidget(highPriorityLabel_);
    statsLayout->addWidget(mediumPriorityLabel_);
    statsLayout->addWidget(lowPriorityLabel_);
    statsLayout->addWidget(adoptionRateLabel_);
    statsLayout->addStretch();

    QPushButton* refreshBtn = new QPushButton("刷新", this);
    refreshBtn->setFixedWidth(80);
    connect(refreshBtn, &QPushButton::clicked, this, &SystemRecommendationsPage::refreshRecommendations);
    statsLayout->addWidget(refreshBtn);

    mainLayout->addLayout(statsLayout);

    table_ = new QTableWidget(this);
    table_->setColumnCount(8);
    table_->setHorizontalHeaderLabels({
        "设备ID", "设备名称", "当前位置", "建议动作", "风险等级", "原因说明", "时间", "操作"
    });

    table_->setSelectionBehavior(QAbstractItemView::SelectRows);
    table_->setSelectionMode(QAbstractItemView::SingleSelection);
    table_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table_->setAlternatingRowColors(true);

    table_->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    table_->horizontalHeader()->setSectionResizeMode(5, QHeaderView::Stretch);

    table_->setColumnWidth(0, 70);
    table_->setColumnWidth(2, 100);
    table_->setColumnWidth(3, 100);
    table_->setColumnWidth(4, 80);
    table_->setColumnWidth(6, 140);
    table_->setColumnWidth(7, 180);

    mainLayout->addWidget(table_);

    QLabel* tipLabel = new QLabel("提示: 系统会根据资产状态自动生成建议。您可以确认执行或忽略建议。", this);
    tipLabel->setStyleSheet("color: #666; font-size: 12px; padding: 5px;");
    mainLayout->addWidget(tipLabel);

    setLayout(mainLayout);
}

void SystemRecommendationsPage::refreshRecommendations() {
    loadRecommendations();
    deduplicateRecommendations();
    updateStats();

    table_->setRowCount(recommendations_.size());

    for (size_t i = 0; i < recommendations_.size(); ++i) {
        const auto& rec = recommendations_[i];

        table_->setItem(i, 0, new QTableWidgetItem(QString::number(rec.asset_id)));
        table_->setItem(i, 1, new QTableWidgetItem(rec.asset_name));
        table_->setItem(i, 2, new QTableWidgetItem(rec.location));
        table_->setItem(i, 3, new QTableWidgetItem(getActionString(rec.action)));
        table_->setItem(i, 4, new QTableWidgetItem(getRiskLevelString(rec.priority)));

        QTableWidgetItem* reasonItem = new QTableWidgetItem(rec.reason);
        reasonItem->setToolTip(rec.reason);
        table_->setItem(i, 5, reasonItem);

        table_->setItem(i, 6, new QTableWidgetItem(rec.timestamp));

        QWidget* buttonWidget = new QWidget(this);
        QHBoxLayout* buttonLayout = new QHBoxLayout(buttonWidget);
        buttonLayout->setContentsMargins(5, 2, 5, 2);

        QPushButton* confirmBtn = new QPushButton("确认执行", this);
        confirmBtn->setFixedWidth(70);
        confirmBtn->setEnabled(!rec.is_handled);
        confirmBtn->setStyleSheet(
            "QPushButton { background-color: #4caf50; color: white; border: none; padding: 5px; border-radius: 3px; }"
            "QPushButton:disabled { background-color: #ccc; }"
            "QPushButton:hover { background-color: #45a049; }"
        );
        connect(confirmBtn, &QPushButton::clicked, this, [this, i]() { onConfirmClicked(i); });

        QPushButton* ignoreBtn = new QPushButton("忽略", this);
        ignoreBtn->setFixedWidth(50);
        ignoreBtn->setEnabled(!rec.is_handled);
        ignoreBtn->setStyleSheet(
            "QPushButton { background-color: #9e9e9e; color: white; border: none; padding: 5px; border-radius: 3px; }"
            "QPushButton:disabled { background-color: #e0e0e0; }"
            "QPushButton:hover { background-color: #757575; }"
        );
        connect(ignoreBtn, &QPushButton::clicked, this, [this, i]() { onIgnoreClicked(i); });

        buttonLayout->addWidget(confirmBtn);
        buttonLayout->addWidget(ignoreBtn);
        buttonLayout->addStretch();

        table_->setCellWidget(i, 7, buttonWidget);

        applyRowStyle(i, rec.priority);
    }

    table_->resizeRowsToContents();
}

void SystemRecommendationsPage::loadRecommendations() {
    recommendations_.clear();

    if (grpcClient_) {
        DecisionsResponse resp = grpcClient_->getRecentDecisions(100, "");
        cachedAdoptionRate_ = resp.adoption_rate;

        for (const auto& d : resp.decisions) {
            RecommendationItem rec;
            rec.asset_id = d.asset_id;
            rec.asset_name = QString::fromStdString(d.asset_name);
            rec.location = QString::fromStdString(d.location);
            rec.action = QString::fromStdString(d.action);
            rec.reason = QString::fromStdString(d.reason);
            rec.timestamp = QString::fromStdString(d.timestamp);
            rec.is_handled = d.is_handled;
            rec.is_executed = d.is_executed;
            rec.is_ignored = d.is_ignored;

            if (d.risk_level == "HIGH") rec.priority = 1;
            else if (d.risk_level == "MEDIUM") rec.priority = 3;
            else rec.priority = 5;

            recommendations_.push_back(rec);
        }
    } else {
        loadMockData();
    }
}

void SystemRecommendationsPage::loadMockData() {
    static int mockAdoptionCount = 0;
    static int mockTotalCount = 0;
    mockTotalCount += 3;
    mockAdoptionCount += 2;
    cachedAdoptionRate_ = mockTotalCount > 0 ? (double)mockAdoptionCount / mockTotalCount * 100 : 0;

    RecommendationItem rec1;
    rec1.asset_id = 1001;
    rec1.asset_name = "Laptop-ThinkPad-X1";
    rec1.location = "研发部-A区";
    rec1.action = "INSPECT";
    rec1.reason = "超过72小时未扫描，丢失风险高";
    rec1.priority = 1;
    rec1.timestamp = QDateTime::currentDateTime().addSecs(-3600).toString("yyyy-MM-dd HH:mm");
    rec1.is_handled = false;
    rec1.is_executed = false;
    rec1.is_ignored = false;
    recommendations_.push_back(rec1);

    RecommendationItem rec2;
    rec2.asset_id = 1002;
    rec2.asset_name = "Printer-HP-LaserJet";
    rec2.location = "办公区-2楼";
    rec2.action = "CHECK_USAGE";
    rec2.reason = "扫描频率异常，过去24小时仅扫描2次";
    rec2.priority = 3;
    rec2.timestamp = QDateTime::currentDateTime().addSecs(-7200).toString("yyyy-MM-dd HH:mm");
    rec2.is_handled = false;
    rec2.is_executed = false;
    rec2.is_ignored = false;
    recommendations_.push_back(rec2);

    RecommendationItem rec3;
    rec3.asset_id = 1003;
    rec3.asset_name = "Monitor-Dell-27inch";
    rec3.location = "会议室-B";
    rec3.action = "RELOCATE";
    rec3.reason = "设备在非授权区域，请确认位置";
    rec3.priority = 2;
    rec3.timestamp = QDateTime::currentDateTime().addSecs(-1800).toString("yyyy-MM-dd HH:mm");
    rec3.is_handled = false;
    rec3.is_executed = false;
    rec3.is_ignored = false;
    recommendations_.push_back(rec3);

    RecommendationItem rec4;
    rec4.asset_id = 1004;
    rec4.asset_name = "Server-Rack-01";
    rec4.location = "机房-主网络柜";
    rec4.action = "NOTIFICATION";
    rec4.reason = "设备温度传感器异常，建议关注";
    rec4.priority = 4;
    rec4.timestamp = QDateTime::currentDateTime().addSecs(-900).toString("yyyy-MM-dd HH:mm");
    rec4.is_handled = true;
    rec4.is_executed = true;
    rec4.is_ignored = false;
    recommendations_.push_back(rec4);
}

void SystemRecommendationsPage::deduplicateRecommendations() {
    std::map<int, RecommendationItem> unique;
    for (const auto& rec : recommendations_) {
        auto it = unique.find(rec.asset_id);
        if (it == unique.end() || rec.priority < it->second.priority) {
            unique[rec.asset_id] = rec;
        }
    }

    recommendations_.clear();
    for (const auto& pair : unique) {
        recommendations_.push_back(pair.second);
    }

    std::sort(recommendations_.begin(), recommendations_.end(), [](const RecommendationItem& a, const RecommendationItem& b) {
        if (a.priority != b.priority) return a.priority < b.priority;
        return a.asset_id < b.asset_id;
    });
}

QString SystemRecommendationsPage::getRiskLevelString(int priority) {
    if (priority <= 2) return "HIGH";
    if (priority <= 3) return "MEDIUM";
    return "LOW";
}

QString SystemRecommendationsPage::getActionString(const QString& action) {
    if (action == "INSPECT") return "立即巡检";
    if (action == "SECURITY_ALERT") return "安全告警";
    if (action == "CHECK_USAGE") return "检查使用";
    if (action == "RELOCATE") return "重新分配";
    if (action == "MAINTENANCE") return "维护检查";
    if (action == "NOTIFICATION") return "关注";
    return "无需操作";
}

QColor SystemRecommendationsPage::getRiskColor(int priority) {
    if (priority <= 2) return QColor(255, 235, 238);
    if (priority <= 3) return QColor(255, 243, 224);
    return QColor(232, 245, 233);
}

void SystemRecommendationsPage::applyRowStyle(int row, int priority) {
    QColor bgColor = getRiskColor(priority);

    for (int col = 0; col < table_->columnCount(); ++col) {
        if (table_->item(row, col)) {
            table_->item(row, col)->setBackground(bgColor);
        }
    }
}

void SystemRecommendationsPage::onConfirmClicked(int row) {
    if (row < 0 || row >= (int)recommendations_.size()) return;

    int assetId = recommendations_[row].asset_id;
    qDebug() << "Confirmed decision for asset:" << assetId << "by user:" << current_user_;

    if (grpcClient_) {
        grpcClient_->reportDecision(assetId, true, false, current_user_.toStdString());
    }

    recommendations_[row].is_handled = true;
    recommendations_[row].is_executed = true;

    refreshRecommendations();
}

void SystemRecommendationsPage::onIgnoreClicked(int row) {
    if (row < 0 || row >= (int)recommendations_.size()) return;

    int assetId = recommendations_[row].asset_id;
    qDebug() << "Ignored decision for asset:" << assetId << "by user:" << current_user_;

    if (grpcClient_) {
        grpcClient_->reportDecision(assetId, false, true, current_user_.toStdString());
    }

    recommendations_[row].is_handled = true;
    recommendations_[row].is_ignored = true;

    refreshRecommendations();
}

void SystemRecommendationsPage::updateStats() {
    int total = recommendations_.size();
    int high = 0, medium = 0, low = 0;

    for (const auto& rec : recommendations_) {
        if (rec.priority <= 2) high++;
        else if (rec.priority <= 3) medium++;
        else low++;
    }

    totalLabel_->setText(QString("总建议数: %1").arg(total));
    highPriorityLabel_->setText(QString("高优先级: %1").arg(high));
    mediumPriorityLabel_->setText(QString("中优先级: %1").arg(medium));
    lowPriorityLabel_->setText(QString("低优先级: %1").arg(low));

    if (cachedAdoptionRate_ > 0) {
        adoptionRateLabel_->setText(QString("采纳率: %1%").arg(cachedAdoptionRate_, 0, 'f', 1));
    }
}
