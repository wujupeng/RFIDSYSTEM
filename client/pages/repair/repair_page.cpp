#include "repair_page.h"
#include "../../network/grpc_client.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QMessageBox>
#include <QInputDialog>

RepairPage::RepairPage(QWidget* parent)
    : QWidget(parent) {

    auto* mainLayout = new QVBoxLayout(this);

    QGroupBox* actionGroup = new QGroupBox("Repair Actions");
    auto* actionLayout = new QHBoxLayout();

    auto* submitBtn = new QPushButton("Submit for Repair");
    auto* completeBtn = new QPushButton("Mark as Repaired");
    auto* scrapBtn = new QPushButton("Scrap Asset");
    auto* refreshBtn = new QPushButton("Refresh");

    actionLayout->addWidget(submitBtn);
    actionLayout->addWidget(completeBtn);
    actionLayout->addWidget(scrapBtn);
    actionLayout->addWidget(refreshBtn);
    actionLayout->addStretch();

    actionGroup->setLayout(actionLayout);

    repairTable_ = new QTableWidget();
    repairTable_->setColumnCount(6);
    repairTable_->setHorizontalHeaderLabels({
        "ID", "Asset Code", "Name", "Current Status", "Location", "Actions"
    });
    repairTable_->horizontalHeader()->setStretchLastSection(true);
    repairTable_->setSelectionBehavior(QAbstractItemView::SelectRows);
    repairTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);

    statusLabel_ = new QLabel();

    mainLayout->addWidget(actionGroup);
    mainLayout->addWidget(new QLabel("Assets in Repair"));
    mainLayout->addWidget(repairTable_);
    mainLayout->addWidget(statusLabel_);

    connect(submitBtn, &QPushButton::clicked, this, &RepairPage::onSubmitRepair);
    connect(completeBtn, &QPushButton::clicked, this, &RepairPage::onCompleteRepair);
    connect(refreshBtn, &QPushButton::clicked, this, &RepairPage::onRefreshRepairList);
    connect(scrapBtn, &QPushButton::clicked, this, &RepairPage::onSubmitRepair);

    onRefreshRepairList();
}

void RepairPage::onSubmitRepair() {
    bool ok;
    int assetId = QInputDialog::getInt(this, "Submit for Repair",
                                        "Enter Asset ID:", 0, 1, 999999, 1, &ok);
    if (!ok) return;

    QString operatorName = QInputDialog::getText(this, "Operator", "Operator Name:",
                                                  QLineEdit::Normal, "system", &ok);
    if (!ok) return;

    GrpcClient client("localhost:50051");
    bool success = client.updateAssetStatus(assetId, "REPAIR", operatorName.toStdString());

    if (success) {
        QMessageBox::information(this, "Success", "Asset submitted for repair");
        onRefreshRepairList();
    } else {
        QMessageBox::warning(this, "Failed", "Could not update asset status");
    }
}

void RepairPage::onCompleteRepair() {
    bool ok;
    int assetId = QInputDialog::getInt(this, "Complete Repair",
                                        "Enter Asset ID:", 0, 1, 999999, 1, &ok);
    if (!ok) return;

    QString operatorName = QInputDialog::getText(this, "Operator", "Operator Name:",
                                                  QLineEdit::Normal, "system", &ok);
    if (!ok) return;

    GrpcClient client("localhost:50051");
    bool success = client.updateAssetStatus(assetId, "IN_STOCK", operatorName.toStdString());

    if (success) {
        QMessageBox::information(this, "Success", "Asset repair completed");
        onRefreshRepairList();
    } else {
        QMessageBox::warning(this, "Failed", "Could not update asset status");
    }
}

void RepairPage::onRefreshRepairList() {
    GrpcClient client("localhost:50051");

    GrpcClient::ListAssetsParams params;
    params.page = 1;
    params.page_size = 100;
    params.status_filter = "REPAIR";

    auto assets = client.listAssets(params);

    repairTable_->setRowCount(0);
    for (const auto& asset : assets) {
        int row = repairTable_->rowCount();
        repairTable_->insertRow(row);

        repairTable_->setItem(row, 0, new QTableWidgetItem(QString::number(asset.id)));
        repairTable_->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(asset.asset_code)));
        repairTable_->setItem(row, 2, new QTableWidgetItem(QString::fromStdString(asset.name)));
        repairTable_->setItem(row, 3, new QTableWidgetItem(QString::fromStdString(asset.status)));
        repairTable_->setItem(row, 4, new QTableWidgetItem(QString::fromStdString(asset.location)));
    }

    statusLabel_->setText(QString("Assets in repair: %1").arg(assets.size()));
}