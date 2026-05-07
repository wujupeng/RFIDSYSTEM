#include "bi_report_page.h"
#include "../../network/bi_client.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QFileDialog>
#include <QDateTime>

BIReportPage::BIReportPage(BIClient* client, QWidget* parent)
    : QWidget(parent), client_(client) {
    setupUi();
    loadReports();
    loadSchedules();
}

void BIReportPage::setupUi() {
    auto* mainLayout = new QVBoxLayout(this);
    
    // Report generation section
    auto* genLayout = new QHBoxLayout();
    
    reportTypeCombo_ = new QComboBox();
    reportTypeCombo_->addItem("日报", 0);
    reportTypeCombo_->addItem("周报", 1);
    reportTypeCombo_->addItem("月报", 2);
    reportTypeCombo_->addItem("自定义", 3);
    
    startDateEdit_ = new QDateEdit(QDate::currentDate().addDays(-7));
    endDateEdit_ = new QDateEdit(QDate::currentDate());
    
    generateBtn_ = new QPushButton("生成报表");
    exportBtn_ = new QPushButton("导出报表");
    exportBtn_->setEnabled(false);
    
    genLayout->addWidget(new QLabel("报表类型:"));
    genLayout->addWidget(reportTypeCombo_);
    genLayout->addWidget(new QLabel("开始日期:"));
    genLayout->addWidget(startDateEdit_);
    genLayout->addWidget(new QLabel("结束日期:"));
    genLayout->addWidget(endDateEdit_);
    genLayout->addWidget(generateBtn_);
    genLayout->addWidget(exportBtn_);
    
    mainLayout->addLayout(genLayout);
    
    // Reports list
    auto* reportsGroup = new QGroupBox("历史报表");
    auto* reportsLayout = new QVBoxLayout(reportsGroup);
    
    reportsTable_ = new QTableWidget();
    reportsTable_->setColumnCount(3);
    reportsTable_->setHorizontalHeaderLabels({"报表ID", "标题", "生成时间"});
    reportsTable_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    
    reportsLayout->addWidget(reportsTable_);
    mainLayout->addWidget(reportsGroup);
    
    // Chart display
    auto* chartGroup = new QGroupBox("报表预览");
    auto* chartLayout = new QVBoxLayout(chartGroup);
    
    reportTitleLabel_ = new QLabel("选择报表查看详情");
    reportTitleLabel_->setStyleSheet("font-weight: bold; font-size: 14px;");
    
    chartView_ = new QChartView();
    chartView_->setMinimumHeight(300);
    
    dataTable_ = new QTableWidget();
    dataTable_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    
    chartLayout->addWidget(reportTitleLabel_);
    chartLayout->addWidget(chartView_);
    chartLayout->addWidget(dataTable_);
    
    mainLayout->addWidget(chartGroup);
    
    // Schedule section
    auto* scheduleGroup = new QGroupBox("定时任务");
    auto* scheduleLayout = new QVBoxLayout(scheduleGroup);
    
    auto* scheduleBtnLayout = new QHBoxLayout();
    addScheduleBtn_ = new QPushButton("添加定时任务");
    scheduleBtnLayout->addWidget(addScheduleBtn_);
    scheduleBtnLayout->addStretch();
    
    schedulesTable_ = new QTableWidget();
    schedulesTable_->setColumnCount(4);
    schedulesTable_->setHorizontalHeaderLabels({"任务ID", "名称", "Cron表达式", "状态"});
    schedulesTable_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    
    scheduleLayout->addLayout(scheduleBtnLayout);
    scheduleLayout->addWidget(schedulesTable_);
    
    mainLayout->addWidget(scheduleGroup);
    
    // Connections
    connect(generateBtn_, &QPushButton::clicked, this, &BIReportPage::onGenerateReport);
    connect(exportBtn_, &QPushButton::clicked, this, &BIReportPage::onExportReport);
    connect(reportsTable_, &QTableWidget::cellDoubleClicked, this, &BIReportPage::onReportSelected);
    connect(addScheduleBtn_, &QPushButton::clicked, this, &BIReportPage::onAddSchedule);
}

void BIReportPage::onGenerateReport() {
    try {
        bi::ReportFilter filter;
        filter.set_start_date(startDateEdit_->date().toString("yyyy-MM-dd").toStdString());
        filter.set_end_date(endDateEdit_->date().toString("yyyy-MM-dd").toStdString());
        
        auto report = client_->generateReport(
            reportTypeCombo_->currentData().toInt(), filter);
        
        if (!report.report_id().empty()) {
            QMessageBox::information(this, "成功", "报表生成成功！");
            loadReports();
            exportBtn_->setEnabled(true);
        }
        
    } catch (const std::exception& e) {
        QMessageBox::warning(this, "错误", QString("生成报表失败: %1").arg(e.what()));
    }
}

void BIReportPage::onExportReport() {
    if (currentReportId_.isEmpty()) {
        QMessageBox::warning(this, "警告", "请先选择一个报表");
        return;
    }
    
    try {
        auto [data, filename] = client_->exportReport(currentReportId_.toStdString(), 0);
        
        QString path = QFileDialog::getSaveFileName(this, "保存报表", 
            QString::fromStdString(filename), "CSV文件 (*.csv)");
        
        if (!path.isEmpty()) {
            QFile file(path);
            if (file.open(QIODevice::WriteOnly)) {
                file.write(data.c_str());
                file.close();
                QMessageBox::information(this, "成功", "报表导出成功！");
            }
        }
        
    } catch (const std::exception& e) {
        QMessageBox::warning(this, "错误", QString("导出报表失败: %1").arg(e.what()));
    }
}

void BIReportPage::onRefreshReports() {
    loadReports();
}

void BIReportPage::onReportSelected(int row) {
    QString reportId = reportsTable_->item(row, 0)->text();
    currentReportId_ = reportId;
    exportBtn_->setEnabled(true);
    
    try {
        auto report = client_->getReport(reportId.toStdString());
        displayReport(report);
        
    } catch (const std::exception& e) {
        QMessageBox::warning(this, "错误", QString("加载报表失败: %1").arg(e.what()));
    }
}

void BIReportPage::onAddSchedule() {
    try {
        bi::ScheduledTask task;
        task.set_name("每日报表任务");
        task.set_report_type(0);
        task.set_cron_expression("0 9 * * *");
        task.set_export_format(0);
        task.set_enabled(true);
        
        std::string taskId = client_->addTask(task);
        
        if (!taskId.empty()) {
            QMessageBox::information(this, "成功", "定时任务添加成功！");
            loadSchedules();
        }
        
    } catch (const std::exception& e) {
        QMessageBox::warning(this, "错误", QString("添加任务失败: %1").arg(e.what()));
    }
}

void BIReportPage::onRefreshSchedules() {
    loadSchedules();
}

void BIReportPage::loadReports() {
    try {
        auto reports = client_->getReports(10);
        
        reportsTable_->setRowCount(0);
        for (const auto& report : reports) {
            int row = reportsTable_->rowCount();
            reportsTable_->insertRow(row);
            
            reportsTable_->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(report.report_id())));
            reportsTable_->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(report.title())));
            reportsTable_->setItem(row, 2, new QTableWidgetItem(QString::fromStdString(report.generated_at())));
        }
        
    } catch (const std::exception& e) {
        QMessageBox::warning(this, "错误", QString("加载报表列表失败: %1").arg(e.what()));
    }
}

void BIReportPage::loadSchedules() {
    try {
        auto tasks = client_->getTasks();
        
        schedulesTable_->setRowCount(0);
        for (const auto& task : tasks) {
            int row = schedulesTable_->rowCount();
            schedulesTable_->insertRow(row);
            
            schedulesTable_->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(task.task_id())));
            schedulesTable_->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(task.name())));
            schedulesTable_->setItem(row, 2, new QTableWidgetItem(QString::fromStdString(task.cron_expression())));
            schedulesTable_->setItem(row, 3, new QTableWidgetItem(task.enabled() ? "启用" : "禁用"));
        }
        
    } catch (const std::exception& e) {
        QMessageBox::warning(this, "错误", QString("加载定时任务失败: %1").arg(e.what()));
    }
}

void BIReportPage::displayReport(const bi::ReportResult& report) {
    reportTitleLabel_->setText(QString::fromStdString(report.title()));
    
    if (report.sections().empty()) {
        dataTable_->setRowCount(0);
        return;
    }
    
    const auto& firstSection = report.sections(0);
    
    // Create chart
    QPieSeries* series = new QPieSeries();
    for (const auto& dp : firstSection.data_points()) {
        series->append(QString::fromStdString(dp.label()), dp.value());
    }
    
    QChart* chart = new QChart();
    chart->addSeries(series);
    chart->setTitle(QString::fromStdString(firstSection.title()));
    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);
    
    chartView_->setChart(chart);
    
    // Display data table
    dataTable_->setRowCount(0);
    dataTable_->setColumnCount(3);
    dataTable_->setHorizontalHeaderLabels({"指标", "数值", "单位"});
    
    for (const auto& dp : firstSection.data_points()) {
        int row = dataTable_->rowCount();
        dataTable_->insertRow(row);
        
        dataTable_->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(dp.label())));
        dataTable_->setItem(row, 1, new QTableWidgetItem(QString::number(dp.value())));
        dataTable_->setItem(row, 2, new QTableWidgetItem(QString::fromStdString(dp.unit())));
    }
}