#include "inventory_scan_page.h"
#include "../../network/grpc_client.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QDateTime>

InventoryScanPage::InventoryScanPage(QWidget* parent)
    : QWidget(parent), isScanning_(false), taskId_(0), pendingRefresh_(false) {

    auto* mainLayout = new QVBoxLayout(this);

    QGroupBox* controlGroup = new QGroupBox("Scan Control");
    auto* controlLayout = new QHBoxLayout();

    epcInput_ = new QLineEdit();
    epcInput_->setPlaceholderText("Enter EPC code or paste multiple EPCs (one per line)");
    epcInput_->setMinimumWidth(400);

    addBtn_ = new QPushButton("Add EPC");
    startBtn_ = new QPushButton("Start Scan");
    stopBtn_ = new QPushButton("Stop Scan");
    clearBtn_ = new QPushButton("Clear All");
    batchScanBtn_ = new QPushButton("Batch Scan");

    stopBtn_->setEnabled(false);

    controlLayout->addWidget(epcInput_);
    controlLayout->addWidget(addBtn_);
    controlLayout->addWidget(startBtn_);
    controlLayout->addWidget(stopBtn_);
    controlLayout->addWidget(clearBtn_);
    controlLayout->addWidget(batchScanBtn_);
    controlGroup->setLayout(controlLayout);

    QGroupBox* statsGroup = new QGroupBox("Scan Statistics");
    auto* statsLayout = new QHBoxLayout();

    scannedCountLabel_ = new QLabel("Scanned: 0");
    foundCountLabel_ = new QLabel("Found: 0");
    foundCountLabel_->setStyleSheet("color: #27ae60; font-weight: bold;");
    missingCountLabel_ = new QLabel("Missing: 0");
    missingCountLabel_->setStyleSheet("color: #e74c3c; font-weight: bold;");
    extraCountLabel_ = new QLabel("Extra: 0");
    extraCountLabel_->setStyleSheet("color: #f39c12; font-weight: bold;");

    statsLayout->addWidget(scannedCountLabel_);
    statsLayout->addWidget(foundCountLabel_);
    statsLayout->addWidget(missingCountLabel_);
    statsLayout->addWidget(extraCountLabel_);
    statsGroup->setLayout(statsLayout);

    scanTable_ = new QTableWidget();
    scanTable_->setColumnCount(4);
    scanTable_->setHorizontalHeaderLabels({"EPC", "Status", "Asset Code", "Asset Name"});
    scanTable_->horizontalHeader()->setStretchLastSection(true);
    scanTable_->setSelectionBehavior(QAbstractItemView::SelectRows);

    refreshTimer_ = new QTimer(this);
    refreshTimer_->setInterval(1000);
    connect(refreshTimer_, &QTimer::timeout, this, &InventoryScanPage::onRefreshTimer);

    mainLayout->addWidget(controlGroup);
    mainLayout->addWidget(statsGroup);
    mainLayout->addWidget(scanTable_);

    connect(addBtn_, &QPushButton::clicked, this, &InventoryScanPage::onAddEPC);
    connect(startBtn_, &QPushButton::clicked, this, &InventoryScanPage::onStartScan);
    connect(stopBtn_, &QPushButton::clicked, this, &InventoryScanPage::onStopScan);
    connect(clearBtn_, &QPushButton::clicked, this, &InventoryScanPage::onClearAll);
    connect(batchScanBtn_, &QPushButton::clicked, this, &InventoryScanPage::onBatchScan);
}

InventoryScanPage::~InventoryScanPage() {
    refreshTimer_->stop();
}

void InventoryScanPage::onStartScan() {
    bool ok;
    QString taskName = QInputDialog::getText(this, "Start Inventory",
                                             "Enter task name:", QLineEdit::Normal,
                                             QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss"), &ok);
    if (!ok || taskName.isEmpty()) return;

    QString location = QInputDialog::getText(this, "Start Inventory",
                                             "Enter location:", QLineEdit::Normal,
                                             "Warehouse A", &ok);
    if (!ok) return;

    GrpcClient client("localhost:50051");
    taskId_ = client.startInventoryTask(taskName.toStdString(), location.toStdString(), "operator");

    if (taskId_ > 0) {
        isScanning_ = true;
        startBtn_->setEnabled(false);
        stopBtn_->setEnabled(true);
        refreshTimer_->start();
        QMessageBox::information(this, "Success", QString("Inventory task started (ID: %1)").arg(taskId_));
    } else {
        QMessageBox::critical(this, "Error", "Failed to start inventory task");
    }
}

void InventoryScanPage::onStopScan() {
    refreshTimer_->stop();

    onRefreshTimer();

    GrpcClient client("localhost:50051");
    client.completeInventoryTask(taskId_);

    isScanning_ = false;
    startBtn_->setEnabled(true);
    stopBtn_->setEnabled(false);

    QMessageBox::information(this, "Success", "Inventory task completed");
}

void InventoryScanPage::onAddEPC() {
    QString epc = epcInput_->text().trimmed();
    if (epc.isEmpty()) return;

    QStringList epcs = epc.split('\n', Qt::SkipEmptyParts);

    GrpcClient client("localhost:50051");
    std::vector<std::string> epcList;
    for (const QString& e : epcs) {
        epcList.push_back(e.trimmed().toStdString());
    }

    if (!epcList.empty()) {
        auto results = client.batchScanEPC(epcList, taskId_);

        for (const auto& result : results.found) {
            queueResult(QString::fromStdString(result.epc), "FOUND",
                       QString("%1 - %2").arg(QString::fromStdString(result.asset_code))
                                         .arg(QString::fromStdString(result.asset_name)));
        }

        for (const auto& epcStr : results.extra) {
            queueResult(QString::fromStdString(epcStr), "EXTRA");
        }
    }

    epcInput_->clear();
}

void InventoryScanPage::onClearAll() {
    scanTable_->setRowCount(0);
    updateStats();
}

void InventoryScanPage::onBatchScan() {
    QString text = QInputDialog::getMultiLineText(this, "Batch Scan",
                                                  "Paste EPC codes (one per line):");
    if (text.isEmpty()) return;

    QStringList epcs = text.split('\n', Qt::SkipEmptyParts);

    GrpcClient client("localhost:50051");
    std::vector<std::string> epcList;
    for (const QString& e : epcs) {
        QString trimmed = e.trimmed();
        if (!trimmed.isEmpty()) {
            epcList.push_back(trimmed.toStdString());
        }
    }

    if (!epcList.empty()) {
        auto results = client.batchScanEPC(epcList, taskId_);

        for (const auto& result : results.found) {
            queueResult(QString::fromStdString(result.epc), "FOUND",
                       QString("%1 - %2").arg(QString::fromStdString(result.asset_code))
                                         .arg(QString::fromStdString(result.asset_name)));
        }

        for (const auto& epcStr : results.extra) {
            queueResult(QString::fromStdString(epcStr), "EXTRA");
        }
    }
}

void InventoryScanPage::queueResult(const QString& epc, const QString& status, const QString& assetInfo) {
    QMutexLocker locker(&resultMutex_);

    PendingResult result;
    result.epc = epc;
    result.status = status;
    result.assetInfo = assetInfo;
    pendingResults_.append(result);
    pendingRefresh_ = true;
}

void InventoryScanPage::onRefreshTimer() {
    QMutexLocker locker(&resultMutex_);

    if (!pendingRefresh_) return;

    for (const auto& result : pendingResults_) {
        addScanResult(result.epc, result.status, result.assetInfo);
    }

    pendingResults_.clear();
    pendingRefresh_ = false;

    updateStats();
}

void InventoryScanPage::addScanResult(const QString& epc, const QString& status, const QString& assetInfo) {
    int row = scanTable_->rowCount();
    scanTable_->insertRow(row);

    scanTable_->setItem(row, 0, new QTableWidgetItem(epc));

    auto* statusItem = new QTableWidgetItem(status);
    if (status == "FOUND") {
        statusItem->setBackground(QColor(39, 174, 96));
        statusItem->setForeground(Qt::white);
    } else if (status == "MISSING") {
        statusItem->setBackground(QColor(231, 76, 60));
        statusItem->setForeground(Qt::white);
    } else if (status == "EXTRA") {
        statusItem->setBackground(QColor(243, 156, 18));
        statusItem->setForeground(Qt::white);
    }
    scanTable_->setItem(row, 1, statusItem);

    if (!assetInfo.isEmpty()) {
        QStringList parts = assetInfo.split(" - ");
        scanTable_->setItem(row, 2, new QTableWidgetItem(parts[0]));
        if (parts.size() > 1) {
            scanTable_->setItem(row, 3, new QTableWidgetItem(parts[1]));
        }
    }
}

void InventoryScanPage::updateStats() {
    int total = scanTable_->rowCount();
    int found = 0, missing = 0, extra = 0;

    for (int i = 0; i < total; ++i) {
        QString status = scanTable_->item(i, 1)->text();
        if (status == "FOUND") found++;
        else if (status == "MISSING") missing++;
        else if (status == "EXTRA") extra++;
    }

    scannedCountLabel_->setText(QString("Scanned: %1").arg(total));
    foundCountLabel_->setText(QString("Found: %1").arg(found));
    missingCountLabel_->setText(QString("Missing: %1").arg(missing));
    extraCountLabel_->setText(QString("Extra: %1").arg(extra));
}