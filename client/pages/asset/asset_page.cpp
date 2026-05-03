#include "asset_page.h"
#include "../../network/grpc_client.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QMessageBox>
#include <QHeaderView>

AssetPage::AssetPage(QWidget* parent)
    : QWidget(parent) {

    auto* mainLayout = new QVBoxLayout(this);

    QGroupBox* formGroup = new QGroupBox("Create New Asset");
    QFormLayout* formLayout = new QFormLayout();

    nameInput_ = new QLineEdit();
    nameInput_->setPlaceholderText("Enter asset name");
    typeInput_ = new QLineEdit();
    typeInput_->setPlaceholderText("Enter asset type");
    assetCodeInput_ = new QLineEdit();
    assetCodeInput_->setPlaceholderText("Auto-generated if empty");
    rfidEpcInput_ = new QLineEdit();
    rfidEpcInput_->setPlaceholderText("RFID EPC code");
    locationInput_ = new QLineEdit();
    locationInput_->setPlaceholderText("Asset location");
    operatorInput_ = new QLineEdit();
    operatorInput_->setPlaceholderText("Operator name");

    formLayout->addRow("Name:", nameInput_);
    formLayout->addRow("Type:", typeInput_);
    formLayout->addRow("Asset Code:", assetCodeInput_);
    formLayout->addRow("RFID EPC:", rfidEpcInput_);
    formLayout->addRow("Location:", locationInput_);
    formLayout->addRow("Operator:", operatorInput_);

    auto* btnLayout = new QHBoxLayout();
    auto* createBtn = new QPushButton("Create Asset");
    auto* clearBtn = new QPushButton("Clear");
    btnLayout->addWidget(createBtn);
    btnLayout->addWidget(clearBtn);
    formLayout->addRow(btnLayout);

    formGroup->setLayout(formLayout);

    assetTable_ = new QTableWidget();
    assetTable_->setColumnCount(7);
    assetTable_->setHorizontalHeaderLabels({"ID", "Asset Code", "Name", "Type", "Location", "Status", "Actions"});
    assetTable_->horizontalHeader()->setStretchLastSection(true);
    assetTable_->setSelectionBehavior(QAbstractItemView::SelectRows);

    statusLabel_ = new QLabel();

    mainLayout->addWidget(formGroup);
    mainLayout->addWidget(new QLabel("Asset List"));
    mainLayout->addWidget(assetTable_);
    mainLayout->addWidget(statusLabel_);

    connect(createBtn, &QPushButton::clicked, this, &AssetPage::onCreateAsset);
    connect(clearBtn, &QPushButton::clicked, this, &AssetPage::clearForm);
    connect(new QPushButton("Refresh"), &QPushButton::clicked, this, &AssetPage::onRefreshList);

    onRefreshList();
}

void AssetPage::onCreateAsset() {
    QString name = nameInput_->text().trimmed();
    QString type = typeInput_->text().trimmed();
    QString operatorName = operatorInput_->text().trimmed();

    if (name.isEmpty() || type.isEmpty()) {
        QMessageBox::warning(this, "Input Error", "Name and type are required");
        return;
    }

    if (operatorName.isEmpty()) {
        operatorName = "system";
    }

    GrpcClient client("localhost:50051");

    GrpcClient::CreateAssetParams params;
    params.name = name.toStdString();
    params.type = type.toStdString();
    params.asset_code = assetCodeInput_->text().trimmed().toStdString();
    params.rfid_epc = rfidEpcInput_->text().trimmed().toStdString();
    params.location = locationInput_->text().trimmed().toStdString();
    params.operator_name = operatorName.toStdString();

    int id = client.createAsset(params);

    if (id > 0) {
        QMessageBox::information(this, "Success", QString("Asset created with ID: %1").arg(id));
        clearForm();
        onRefreshList();
    } else {
        QMessageBox::critical(this, "Error", "Failed to create asset");
    }
}

void AssetPage::onUpdateStatus() {
}

void AssetPage::onRefreshList() {
    GrpcClient client("localhost:50051");

    GrpcClient::ListAssetsParams params;
    params.page = 1;
    params.page_size = 100;

    auto assets = client.listAssets(params);

    assetTable_->setRowCount(0);
    for (const auto& asset : assets) {
        int row = assetTable_->rowCount();
        assetTable_->insertRow(row);

        assetTable_->setItem(row, 0, new QTableWidgetItem(QString::number(asset.id)));
        assetTable_->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(asset.asset_code)));
        assetTable_->setItem(row, 2, new QTableWidgetItem(QString::fromStdString(asset.name)));
        assetTable_->setItem(row, 3, new QTableWidgetItem(QString::fromStdString(asset.type)));
        assetTable_->setItem(row, 4, new QTableWidgetItem(QString::fromStdString(asset.location)));
        assetTable_->setItem(row, 5, new QTableWidgetItem(QString::fromStdString(asset.status)));
    }

    statusLabel_->setText(QString("Total: %1 assets").arg(assets.size()));
}

void AssetPage::clearForm() {
    nameInput_->clear();
    typeInput_->clear();
    assetCodeInput_->clear();
    rfidEpcInput_->clear();
    locationInput_->clear();
}