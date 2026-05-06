#include "allocation_page.h"
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
#include <QTextEdit>
#include <QInputDialog>

AllocationPage::AllocationPage(QWidget* parent)
    : QWidget(parent) {
    setupUi();
    onRefreshList();
}

void AllocationPage::setupUi() {
    auto* mainLayout = new QVBoxLayout(this);

    auto* topLayout = new QHBoxLayout();
    searchEdit_ = new QLineEdit();
    searchEdit_->setPlaceholderText("搜索资产编号或人员...");
    
    auto* searchBtn = new QPushButton("搜索");
    auto* applyBtn = new QPushButton("+ 领用申请");
    auto* approveBtn = new QPushButton("审批通过");
    auto* returnBtn = new QPushButton("归还资产");
    auto* refreshBtn = new QPushButton("刷新");
    
    topLayout->addWidget(searchEdit_);
    topLayout->addWidget(searchBtn);
    topLayout->addStretch();
    topLayout->addWidget(applyBtn);
    topLayout->addWidget(approveBtn);
    topLayout->addWidget(returnBtn);
    topLayout->addWidget(refreshBtn);
    
    allocationTable_ = new QTableWidget();
    allocationTable_->setColumnCount(8);
    allocationTable_->setHorizontalHeaderLabels({
        "ID", "资产编号", "资产名称", "申请人", "审批状态", "领用人员", "领用日期", "状态"
    });
    allocationTable_->horizontalHeader()->setStretchLastSection(true);
    allocationTable_->setSelectionBehavior(QAbstractItemView::SelectRows);
    
    mainLayout->addLayout(topLayout);
    mainLayout->addWidget(allocationTable_);
    
    connect(applyBtn, &QPushButton::clicked, this, &AllocationPage::showApplyDialog);
    connect(approveBtn, &QPushButton::clicked, this, &AllocationPage::onApproveAllocation);
    connect(returnBtn, &QPushButton::clicked, this, &AllocationPage::onReturnAsset);
    connect(refreshBtn, &QPushButton::clicked, this, &AllocationPage::onRefreshList);
    connect(searchBtn, &QPushButton::clicked, this, &AllocationPage::onRefreshList);
}

void AllocationPage::showApplyDialog() {
    QDialog dialog(this);
    dialog.setWindowTitle("领用申请");
    dialog.resize(400, 350);
    
    auto* layout = new QFormLayout(&dialog);
    
    QLineEdit* assetCodeEdit = new QLineEdit();
    assetCodeEdit->setPlaceholderText("资产编号");
    
    QLineEdit* applicantEdit = new QLineEdit();
    applicantEdit->setPlaceholderText("申请人");
    
    QLineEdit* userEdit = new QLineEdit();
    userEdit->setPlaceholderText("领用人员");
    
    QTextEdit* reasonEdit = new QTextEdit();
    reasonEdit->setPlaceholderText("领用原因");
    reasonEdit->setMaximumHeight(80);
    
    layout->addRow("资产编号:", assetCodeEdit);
    layout->addRow("申请人:", applicantEdit);
    layout->addRow("领用人员:", userEdit);
    layout->addRow("领用原因:", reasonEdit);
    
    auto* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    layout->addRow(buttonBox);
    
    connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    
    if (dialog.exec() == QDialog::Accepted) {
        QMessageBox::information(this, "成功", 
            QString("领用申请已提交！\n资产编号: %1\n申请人: %2\n领用人员: %3").arg(
                assetCodeEdit->text(), applicantEdit->text(), userEdit->text()));
        onRefreshList();
    }
}

void AllocationPage::onApproveAllocation() {
    auto selected = allocationTable_->selectedItems();
    if (selected.isEmpty()) {
        QMessageBox::warning(this, "提示", "请选择要审批的申请");
        return;
    }
    
    QString assetCode = selected[1]->text();
    QString applicant = selected[3]->text();
    
    if (QMessageBox::question(this, "确认审批", 
        QString("确定要批准 %1 的领用申请吗？\n资产编号: %2").arg(applicant, assetCode)) == QMessageBox::Yes) {
        
        QMessageBox::information(this, "成功", "审批通过，资产已领用");
        onRefreshList();
    }
}

void AllocationPage::onReturnAsset() {
    auto selected = allocationTable_->selectedItems();
    if (selected.isEmpty()) {
        QMessageBox::warning(this, "提示", "请选择要归还的资产");
        return;
    }
    
    QString assetCode = selected[1]->text();
    QString user = selected[5]->text();
    
    QString condition = QInputDialog::getText(this, "归还验收", "资产状况:", QLineEdit::Normal, "良好");
    
    if (!condition.isEmpty()) {
        QMessageBox::information(this, "成功", 
            QString("资产归还成功！\n资产编号: %1\n使用人员: %2\n状况: %3").arg(assetCode, user, condition));
        onRefreshList();
    }
}

void AllocationPage::onRefreshList() {
    allocationTable_->setRowCount(0);
    
    QStringList statusList = {"审批中", "已通过", "已拒绝"};
    QStringList assetList = {"IT-SZ-2026-0001", "IT-SZ-2026-0002", "IT-SZ-2026-0003", "IT-SZ-2026-0004", "IT-SZ-2026-0005"};
    QStringList nameList = {"笔记本电脑", "显示器", "服务器", "打印机", "网络交换机"};
    QStringList userList = {"张三", "李四", "王五", "赵六", "钱七"};
    
    for (int i = 0; i < 6; ++i) {
        int row = allocationTable_->rowCount();
        allocationTable_->insertRow(row);
        
        allocationTable_->setItem(row, 0, new QTableWidgetItem(QString::number(i + 1)));
        allocationTable_->setItem(row, 1, new QTableWidgetItem(assetList[i % assetList.size()]));
        allocationTable_->setItem(row, 2, new QTableWidgetItem(nameList[i % nameList.size()]));
        allocationTable_->setItem(row, 3, new QTableWidgetItem(userList[i % userList.size()]));
        allocationTable_->setItem(row, 4, new QTableWidgetItem(statusList[i % statusList.size()]));
        allocationTable_->setItem(row, 5, new QTableWidgetItem(statusList[i % statusList.size()] == "已通过" ? userList[(i + 1) % userList.size()] : "-"));
        allocationTable_->setItem(row, 6, new QTableWidgetItem(statusList[i % statusList.size()] == "已通过" ? "2026-05-06" : "-"));
        allocationTable_->setItem(row, 7, new QTableWidgetItem(statusList[i % statusList.size()] == "已通过" ? "使用中" : "在库"));
    }
}
