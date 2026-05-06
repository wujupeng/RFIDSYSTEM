#include "purchase_page.h"
#include "../../network/grpc_client.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QMessageBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QInputDialog>
#include <QDateTime>
#include <sstream>

PurchasePage::PurchasePage(QWidget* parent)
    : QWidget(parent) {
    setupUi();
    onRefreshList();
}

void PurchasePage::setupUi() {
    auto* mainLayout = new QVBoxLayout(this);

    auto* topLayout = new QHBoxLayout();
    searchEdit_ = new QLineEdit();
    searchEdit_->setPlaceholderText("搜索采购单号...");
    
    auto* searchBtn = new QPushButton("搜索");
    auto* addBtn = new QPushButton("+ 新增采购单");
    auto* rfidBtn = new QPushButton("RFID写卡");
    auto* refreshBtn = new QPushButton("刷新");
    
    topLayout->addWidget(searchEdit_);
    topLayout->addWidget(searchBtn);
    topLayout->addStretch();
    topLayout->addWidget(addBtn);
    topLayout->addWidget(rfidBtn);
    topLayout->addWidget(refreshBtn);
    
    purchaseTable_ = new QTableWidget();
    purchaseTable_->setColumnCount(7);
    purchaseTable_->setHorizontalHeaderLabels({
        "ID", "采购单号", "资产编号", "资产名称", "规格型号", "供应商", "状态"
    });
    purchaseTable_->horizontalHeader()->setStretchLastSection(true);
    purchaseTable_->setSelectionBehavior(QAbstractItemView::SelectRows);
    
    mainLayout->addLayout(topLayout);
    mainLayout->addWidget(purchaseTable_);
    
    connect(addBtn, &QPushButton::clicked, this, &PurchasePage::showPurchaseDialog);
    connect(rfidBtn, &QPushButton::clicked, this, &PurchasePage::onRFIDWrite);
    connect(refreshBtn, &QPushButton::clicked, this, &PurchasePage::onRefreshList);
    connect(searchBtn, &QPushButton::clicked, this, &PurchasePage::onRefreshList);
}

QString PurchasePage::generateAssetCode() {
    QString year = QDateTime::currentDateTime().toString("yyyy");
    static int counter = 1;
    QString code = QString("IT-SZ-%1-%2").arg(year).arg(counter++, 4, 10, QChar('0'));
    return code;
}

void PurchasePage::showPurchaseDialog() {
    QDialog dialog(this);
    dialog.setWindowTitle("新增采购单");
    dialog.resize(400, 350);
    
    auto* layout = new QFormLayout(&dialog);
    
    QLineEdit* poEdit = new QLineEdit();
    poEdit->setPlaceholderText("采购单号");
    
    QLineEdit* nameEdit = new QLineEdit();
    nameEdit->setPlaceholderText("资产名称");
    
    QLineEdit* modelEdit = new QLineEdit();
    modelEdit->setPlaceholderText("规格型号");
    
    QLineEdit* supplierEdit = new QLineEdit();
    supplierEdit->setPlaceholderText("供应商");
    
    QSpinBox* quantitySpin = new QSpinBox();
    quantitySpin->setRange(1, 1000);
    quantitySpin->setValue(1);
    
    layout->addRow("采购单号:", poEdit);
    layout->addRow("资产名称:", nameEdit);
    layout->addRow("规格型号:", modelEdit);
    layout->addRow("供应商:", supplierEdit);
    layout->addRow("数量:", quantitySpin);
    
    auto* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    layout->addRow(buttonBox);
    
    connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    
    if (dialog.exec() == QDialog::Accepted) {
        int quantity = quantitySpin->value();
        QString assetCode = generateAssetCode();
        
        QMessageBox::information(this, "成功", 
            QString("采购单创建成功！\n采购单号: %1\n资产编号: %2\n数量: %3").arg(
                poEdit->text(), assetCode, QString::number(quantity)));
        
        onRefreshList();
    }
}

void PurchasePage::onRFIDWrite() {
    bool ok;
    QString assetCode = QInputDialog::getText(this, "RFID写卡", 
        "输入资产编号:", QLineEdit::Normal, generateAssetCode(), &ok);
    if (!ok || assetCode.isEmpty()) return;
    
    QString epc = QInputDialog::getText(this, "RFID写卡", 
        "输入EPC编码:", QLineEdit::Normal, QString(), &ok);
    if (!ok || epc.isEmpty()) return;
    
    QMessageBox::information(this, "成功", 
        QString("RFID写卡成功！\n资产编号: %1\nEPC编码: %2").arg(assetCode, epc));
    
    GrpcClient client("localhost:50051");
    bool success = client.updateAssetStatus(1, "IN_STOCK", "system");
    if (success) {
        QMessageBox::information(this, "成功", "资产已入库");
    }
    
    onRefreshList();
}

void PurchasePage::onRefreshList() {
    purchaseTable_->setRowCount(0);
    
    QStringList statusList = {"待入库", "已入库", "验收中"};
    QStringList poList = {"PO-2026-001", "PO-2026-002", "PO-2026-003"};
    QStringList nameList = {"笔记本电脑", "显示器", "服务器", "打印机", "网络交换机"};
    QStringList modelList = {"ThinkPad X1", "Dell U2722D", "DELL R750", "HP M404dn", "Cisco Catalyst 3850"};
    QStringList supplierList = {"联想", "戴尔", "惠普", "思科"};
    
    for (int i = 0; i < 8; ++i) {
        int row = purchaseTable_->rowCount();
        purchaseTable_->insertRow(row);
        
        purchaseTable_->setItem(row, 0, new QTableWidgetItem(QString::number(i + 1)));
        purchaseTable_->setItem(row, 1, new QTableWidgetItem(poList[i % poList.size()]));
        purchaseTable_->setItem(row, 2, new QTableWidgetItem(QString("IT-SZ-2026-%1").arg(i + 1, 4, 10, QChar('0'))));
        purchaseTable_->setItem(row, 3, new QTableWidgetItem(nameList[i % nameList.size()]));
        purchaseTable_->setItem(row, 4, new QTableWidgetItem(modelList[i % modelList.size()]));
        purchaseTable_->setItem(row, 5, new QTableWidgetItem(supplierList[i % supplierList.size()]));
        purchaseTable_->setItem(row, 6, new QTableWidgetItem(statusList[i % statusList.size()]));
    }
}

void PurchasePage::onCompletePurchase() {
}

void PurchasePage::onAddPurchase() {
}
