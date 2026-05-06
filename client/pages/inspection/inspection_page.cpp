#include "inspection_page.h"
#include "../../network/grpc_client.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QMessageBox>
#include <QInputDialog>
#include <QDateTime>

InspectionPage::InspectionPage(QWidget* parent)
    : QWidget(parent) {
    setupUi();
    onRefreshList();
}

void InspectionPage::setupUi() {
    auto* mainLayout = new QVBoxLayout(this);

    auto* topLayout = new QHBoxLayout();
    searchEdit_ = new QLineEdit();
    searchEdit_->setPlaceholderText("搜索资产编号...");
    
    auto* searchBtn = new QPushButton("搜索");
    auto* startBtn = new QPushButton("开始巡检");
    auto* scanBtn = new QPushButton("扫描标签");
    auto* completeBtn = new QPushButton("完成路线");
    auto* refreshBtn = new QPushButton("刷新");
    
    topLayout->addWidget(searchEdit_);
    topLayout->addWidget(searchBtn);
    topLayout->addStretch();
    topLayout->addWidget(startBtn);
    topLayout->addWidget(scanBtn);
    topLayout->addWidget(completeBtn);
    topLayout->addWidget(refreshBtn);

    QGroupBox* routeGroup = new QGroupBox("巡检路线");
    auto* routeLayout = new QVBoxLayout(routeGroup);
    
    routeTable_ = new QTableWidget();
    routeTable_->setColumnCount(4);
    routeTable_->setHorizontalHeaderLabels({"路线名称", "站点数", "状态", "操作"});
    routeTable_->horizontalHeader()->setStretchLastSection(true);
    routeLayout->addWidget(routeTable_);

    QGroupBox* inspectionGroup = new QGroupBox("巡检记录");
    auto* inspectionLayout = new QVBoxLayout(inspectionGroup);
    
    inspectionTable_ = new QTableWidget();
    inspectionTable_->setColumnCount(6);
    inspectionTable_->setHorizontalHeaderLabels({
        "ID", "资产编号", "资产名称", "位置", "巡检时间", "状态"
    });
    inspectionTable_->horizontalHeader()->setStretchLastSection(true);
    inspectionTable_->setSelectionBehavior(QAbstractItemView::SelectRows);
    inspectionLayout->addWidget(inspectionTable_);
    
    mainLayout->addLayout(topLayout);
    mainLayout->addWidget(routeGroup);
    mainLayout->addWidget(inspectionGroup);
    
    connect(startBtn, &QPushButton::clicked, this, &InspectionPage::onStartInspection);
    connect(scanBtn, &QPushButton::clicked, this, &InspectionPage::onScanAsset);
    connect(completeBtn, &QPushButton::clicked, this, &InspectionPage::onCompleteRoute);
    connect(refreshBtn, &QPushButton::clicked, this, &InspectionPage::onRefreshList);
    connect(searchBtn, &QPushButton::clicked, this, &InspectionPage::onRefreshList);
}

void InspectionPage::onStartInspection() {
    QStringList routes = {"A区巡检路线", "B区巡检路线", "C区巡检路线", "全厂区巡检"};
    
    bool ok;
    QString selectedRoute = QInputDialog::getItem(this, "选择巡检路线", 
        "请选择巡检路线:", routes, 0, false, &ok);
    
    if (ok && !selectedRoute.isEmpty()) {
        QMessageBox::information(this, "成功", 
            QString("巡检路线已开始：%1").arg(selectedRoute));
        onRefreshList();
    }
}

void InspectionPage::onScanAsset() {
    bool ok;
    QString epc = QInputDialog::getText(this, "扫描标签", 
        "扫描RFID标签（输入EPC编码）:", QLineEdit::Normal, QString(), &ok);
    
    if (ok && !epc.isEmpty()) {
        QString assetCode = "IT-SZ-2026-" + epc.right(4);
        QString time = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
        
        QMessageBox::information(this, "扫描成功", 
            QString("资产已记录！\nEPC: %1\n资产编号: %2\n时间: %3").arg(epc, assetCode, time));
        
        onRefreshList();
    }
}

void InspectionPage::onCompleteRoute() {
    QMessageBox::information(this, "成功", "巡检路线已完成！\n生成巡检报告...");
    onRefreshList();
}

void InspectionPage::onRefreshList() {
    routeTable_->setRowCount(0);
    
    QStringList routeNames = {"A区巡检路线", "B区巡检路线", "C区巡检路线", "全厂区巡检"};
    QStringList statusList = {"未开始", "进行中", "已完成"};
    int stations[] = {15, 20, 18, 53};
    
    for (int i = 0; i < 4; ++i) {
        int row = routeTable_->rowCount();
        routeTable_->insertRow(row);
        
        routeTable_->setItem(row, 0, new QTableWidgetItem(routeNames[i]));
        routeTable_->setItem(row, 1, new QTableWidgetItem(QString::number(stations[i])));
        routeTable_->setItem(row, 2, new QTableWidgetItem(statusList[i % statusList.size()]));
        routeTable_->setItem(row, 3, new QTableWidgetItem("开始"));
    }

    inspectionTable_->setRowCount(0);
    
    QStringList assetList = {"IT-SZ-2026-0001", "IT-SZ-2026-0002", "IT-SZ-2026-0003", "IT-SZ-2026-0004", "IT-SZ-2026-0005"};
    QStringList nameList = {"服务器", "网络交换机", "UPS电源", "空调", "门禁系统"};
    QStringList locationList = {"A栋1层机房", "A栋2层机房", "B栋1层", "B栋2层", "C栋1层"};
    
    for (int i = 0; i < 8; ++i) {
        int row = inspectionTable_->rowCount();
        inspectionTable_->insertRow(row);
        
        inspectionTable_->setItem(row, 0, new QTableWidgetItem(QString::number(i + 1)));
        inspectionTable_->setItem(row, 1, new QTableWidgetItem(assetList[i % assetList.size()]));
        inspectionTable_->setItem(row, 2, new QTableWidgetItem(nameList[i % nameList.size()]));
        inspectionTable_->setItem(row, 3, new QTableWidgetItem(locationList[i % locationList.size()]));
        inspectionTable_->setItem(row, 4, new QTableWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss")));
        inspectionTable_->setItem(row, 5, new QTableWidgetItem("正常"));
    }
}
