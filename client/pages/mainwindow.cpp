#include "mainwindow.h"
#include "dashboard/dashboard_page.h"
#include "asset/asset_page.h"
#include "inventory/inventory_page.h"
#include "repair/repair_page.h"
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QStyle>

MainWindow::MainWindow() {
    setWindowTitle("RFID Asset Management System");
    resize(1200, 800);

    QMenuBar* menuBar = new QMenuBar(this);
    setMenuBar(menuBar);

    QMenu* pagesMenu = menuBar->addMenu("Pages");

    QAction* dashboardAction = pagesMenu->addAction("Dashboard");
    QAction* assetAction = pagesMenu->addAction("Asset Management");
    QAction* inventoryAction = pagesMenu->addAction("Inventory");
    QAction* repairAction = pagesMenu->addAction("Repair");

    stackedWidget_ = new QStackedWidget(this);

    stackedWidget_->addWidget(new DashboardPage());
    stackedWidget_->addWidget(new AssetPage());
    stackedWidget_->addWidget(new InventoryPage());
    stackedWidget_->addWidget(new RepairPage());

    setCentralWidget(stackedWidget_);

    connect(dashboardAction, &QAction::triggered, this, [this]() { stackedWidget_->setCurrentIndex(0); });
    connect(assetAction, &QAction::triggered, this, [this]() { stackedWidget_->setCurrentIndex(1); });
    connect(inventoryAction, &QAction::triggered, this, [this]() { stackedWidget_->setCurrentIndex(2); });
    connect(repairAction, &QAction::triggered, this, [this]() { stackedWidget_->setCurrentIndex(3); });
}