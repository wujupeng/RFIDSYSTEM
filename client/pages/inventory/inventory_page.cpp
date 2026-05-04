#include "inventory_page.h"
#include "../../network/grpc_client.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>

InventoryPage::InventoryPage(QWidget* parent)
    : QWidget(parent) {

    auto* mainLayout = new QVBoxLayout(this);

    auto* filterLayout = new QHBoxLayout();

    searchInput_ = new QLineEdit();
    searchInput_->setPlaceholderText("Search by name, code, or RFID...");

    statusFilter_ = new QComboBox();
    statusFilter_->addItem("All Status", "");
    statusFilter_->addItem("In Stock", "IN_STOCK");
    statusFilter_->addItem("In Use", "IN_USE");
    statusFilter_->addItem("Repair", "REPAIR");
    statusFilter_->addItem("Scrapped", "SCRAPPED");

    auto* searchBtn = new QPushButton("Search");
    auto* exportBtn = new QPushButton("Export CSV");
    auto* refreshBtn = new QPushButton("Refresh");

    filterLayout->addWidget(searchInput_);
    filterLayout->addWidget(statusFilter_);
    filterLayout->addWidget(searchBtn);
    filterLayout->addWidget(exportBtn);
    filterLayout->addWidget(refreshBtn);

    inventoryTable_ = new QTableWidget();
    inventoryTable_->setColumnCount(8);
    inventoryTable_->setHorizontalHeaderLabels({
        "ID", "Asset Code", "Name", "Type", "RFID EPC", "Location", "Status", "Created"
    });
    inventoryTable_->horizontalHeader()->setStretchLastSection(true);
    inventoryTable_->setSelectionBehavior(QAbstractItemView::SelectRows);
    inventoryTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);

    totalLabel_ = new QLabel("Total: 0 items");

    mainLayout->addLayout(filterLayout);
    mainLayout->addWidget(inventoryTable_);
    mainLayout->addWidget(totalLabel_);

    connect(searchBtn, &QPushButton::clicked, this, &InventoryPage::onSearch);
    connect(exportBtn, &QPushButton::clicked, this, &InventoryPage::onExport);
    connect(statusFilter_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &InventoryPage::onFilterChanged);
    connect(refreshBtn, &QPushButton::clicked, this, &InventoryPage::onSearch);

    onSearch();
}

void InventoryPage::onSearch() {
    GrpcClient client("localhost:50051");

    GrpcClient::ListAssetsParams params;
    params.page = 1;
    params.page_size = 1000;
    params.status_filter = statusFilter_->currentData().toString().toStdString();

    auto assets = client.listAssets(params);

    inventoryTable_->setRowCount(0);
    for (const auto& asset : assets) {
        int row = inventoryTable_->rowCount();
        inventoryTable_->insertRow(row);

        inventoryTable_->setItem(row, 0, new QTableWidgetItem(QString::number(asset.id)));
        inventoryTable_->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(asset.asset_code)));
        inventoryTable_->setItem(row, 2, new QTableWidgetItem(QString::fromStdString(asset.name)));
        inventoryTable_->setItem(row, 3, new QTableWidgetItem(QString::fromStdString(asset.type)));
        inventoryTable_->setItem(row, 4, new QTableWidgetItem(QString::fromStdString(asset.rfid_epc)));
        inventoryTable_->setItem(row, 5, new QTableWidgetItem(QString::fromStdString(asset.location)));
        inventoryTable_->setItem(row, 6, new QTableWidgetItem(QString::fromStdString(asset.status)));
    }

    totalLabel_->setText(QString("Total: %1 items").arg(assets.size()));
}

void InventoryPage::onExport() {
    QMessageBox::information(this, "Export", "Export functionality not yet implemented", QMessageBox::Ok);
}

void InventoryPage::onFilterChanged(int) {
    onSearch();
}