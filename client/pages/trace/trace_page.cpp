#include "trace_page.h"
#include <QMessageBox>
#include <QDateTime>

TracePage::TracePage(TrajectoryClient* client, QWidget* parent) 
    : QWidget(parent), client(client), currentPointIndex(0) {
    
    setLayout(new QVBoxLayout(this));
    
    QWidget* headerWidget = new QWidget(this);
    QHBoxLayout* headerLayout = new QHBoxLayout(headerWidget);
    
    QLabel* assetLabel = new QLabel("设备ID:", this);
    assetIdEdit = new QLineEdit(this);
    assetIdEdit->setPlaceholderText("输入设备ID");
    
    searchBtn = new QPushButton("查询轨迹", this);
    playBtn = new QPushButton("播放", this);
    stopBtn = new QPushButton("停止", this);
    resetBtn = new QPushButton("重置", this);
    
    headerLayout->addWidget(assetLabel);
    headerLayout->addWidget(assetIdEdit);
    headerLayout->addWidget(searchBtn);
    headerLayout->addWidget(playBtn);
    headerLayout->addWidget(stopBtn);
    headerLayout->addWidget(resetBtn);
    
    layout()->addWidget(headerWidget);
    
    trajectoryView = new TrajectoryView(this);
    layout()->addWidget(trajectoryView);
    
    QWidget* infoWidget = new QWidget(this);
    QHBoxLayout* infoLayout = new QHBoxLayout(infoWidget);
    
    statusLabel = new QLabel("状态: 就绪", this);
    assetNameLabel = new QLabel("设备: -", this);
    currentLocationLabel = new QLabel("当前位置: -", this);
    riskLabel = new QLabel("风险: -", this);
    
    infoLayout->addWidget(statusLabel);
    infoLayout->addWidget(assetNameLabel);
    infoLayout->addWidget(currentLocationLabel);
    infoLayout->addWidget(riskLabel);
    
    layout()->addWidget(infoWidget);
    
    QWidget* timelineWidget = new QWidget(this);
    QHBoxLayout* timelineLayout = new QHBoxLayout(timelineWidget);
    
    QLabel* timelineLabel = new QLabel("时间轴:", this);
    progressBar = new QProgressBar(this);
    progressBar->setRange(0, 100);
    progressBar->setValue(0);
    
    timelineLayout->addWidget(timelineLabel);
    timelineLayout->addWidget(progressBar);
    
    layout()->addWidget(timelineWidget);
    
    connect(searchBtn, &QPushButton::clicked, this, &TracePage::onSearch);
    connect(playBtn, &QPushButton::clicked, this, &TracePage::onPlay);
    connect(stopBtn, &QPushButton::clicked, this, &TracePage::onStop);
    connect(resetBtn, &QPushButton::clicked, this, &TracePage::onReset);
    connect(trajectoryView, &TrajectoryView::animationFinished, this, &TracePage::onAnimationFinished);
    connect(trajectoryView, &TrajectoryView::pointReached, this, &TracePage::onPointReached);
    
    timeAxisTimer = new QTimer(this);
    connect(timeAxisTimer, &QTimer::timeout, this, &TracePage::updateTimeAxis);
}

TracePage::~TracePage() {
    timeAxisTimer->stop();
    delete timeAxisTimer;
}

void TracePage::onSearch() {
    bool ok;
    int assetId = assetIdEdit->text().toInt(&ok);
    
    if (!ok || assetId <= 0) {
        QMessageBox::warning(this, "错误", "请输入有效的设备ID");
        return;
    }
    
    updateStatus("正在查询轨迹...");
    
    try {
        trajectory::TrajectoryResponse response = client->getTrajectory(assetId);
        
        currentTrajectory.clear();
        for (int i = 0; i < response.points_size(); ++i) {
            currentTrajectory.push_back(response.points(i));
        }
        
        trajectoryView->setTrajectory(currentTrajectory);
        
        updateAssetInfo(assetId, QString("Asset-%1").arg(assetId));
        updateStatus("轨迹加载完成");
        
        if (!currentTrajectory.empty()) {
            QString loc = QString::fromStdString(currentTrajectory.front().location());
            currentLocationLabel->setText("当前位置: " + loc);
            
            double risk = currentTrajectory.front().risk_score();
            QString riskText = QString("风险: %1%").arg(risk * 100, 0, 'f', 1);
            riskLabel->setText(riskText);
        }
        
        progressBar->setValue(0);
        
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "错误", QString("查询失败: %1").arg(e.what()));
        updateStatus("查询失败");
    }
}

void TracePage::onPlay() {
    if (currentTrajectory.empty()) {
        QMessageBox::warning(this, "提示", "请先查询轨迹");
        return;
    }
    
    updateStatus("播放中...");
    trajectoryView->playAnimation();
    timeAxisTimer->start(1500);
}

void TracePage::onStop() {
    trajectoryView->stopAnimation();
    timeAxisTimer->stop();
    updateStatus("已停止");
}

void TracePage::onReset() {
    trajectoryView->resetView();
    timeAxisTimer->stop();
    progressBar->setValue(0);
    currentTrajectory.clear();
    currentPointIndex = 0;
    assetNameLabel->setText("设备: -");
    currentLocationLabel->setText("当前位置: -");
    riskLabel->setText("风险: -");
    updateStatus("已重置");
}

void TracePage::onAnimationFinished() {
    timeAxisTimer->stop();
    progressBar->setValue(100);
    updateStatus("播放完成");
}

void TracePage::onPointReached(const trajectory::TrajectoryPoint& point) {
    QString loc = QString::fromStdString(point.location());
    currentLocationLabel->setText("当前位置: " + loc);
    
    double risk = point.risk_score();
    QString riskText = QString("风险: %1%").arg(risk * 100, 0, 'f', 1);
    riskLabel->setText(riskText);
    
    if (risk > 0.8) {
        riskLabel->setStyleSheet("color: red; font-weight: bold;");
    } else if (risk > 0.5) {
        riskLabel->setStyleSheet("color: orange;");
    } else {
        riskLabel->setStyleSheet("color: green;");
    }
}

void TracePage::updateTimeAxis() {
    if (currentTrajectory.empty()) return;
    
    currentPointIndex++;
    int progress = (currentPointIndex * 100) / currentTrajectory.size();
    progressBar->setValue(qMin(progress, 100));
}

void TracePage::updateStatus(const QString& status) {
    statusLabel->setText("状态: " + status);
}

void TracePage::updateAssetInfo(int assetId, const QString& name) {
    assetNameLabel->setText("设备: " + name);
}