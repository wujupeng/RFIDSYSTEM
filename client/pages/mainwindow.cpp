#include "mainwindow.h"
#include "dashboard/dashboard_page.h"
#include "asset/asset_page.h"
#include "inventory/inventory_page.h"
#include "repair/repair_page.h"
#include "recommendations/system_recommendations_page.h"

MainWindow::MainWindow() {
    setWindowTitle("RFID Asset Management System v2.3");
    resize(1400, 900);

    tabWidget_ = new QTabWidget(this);
    tabWidget_->setTabPosition(QTabWidget::North);
    tabWidget_->setStyleSheet(R"(
        QTabWidget::pane { border: none; }
        QTabBar::tab {
            background: #e0e0e0;
            padding: 10px 20px;
            border: 1px solid #bdbdbd;
            border-bottom: none;
            border-top-left-radius: 5px;
            border-top-right-radius: 5px;
            font-size: 14px;
        }
        QTabBar::tab:selected {
            background: #2196f3;
            color: white;
            border-bottom: 2px solid #1976d2;
        }
        QTabBar::tab:hover:!selected {
            background: #bdbdbd;
        }
    )");

    tabWidget_->addTab(new DashboardPage(), "📊 仪表盘");
    tabWidget_->addTab(new AssetPage(), "💻 资产管理");
    tabWidget_->addTab(new InventoryPage(), "🔍 盘点");
    tabWidget_->addTab(new RepairPage(), "🔧 维修");
    tabWidget_->addTab(new SystemRecommendationsPage(), "⚠️ 系统建议");

    setCentralWidget(tabWidget_);

    connect(tabWidget_, &QTabWidget::currentChanged, this, &MainWindow::onPageChanged);
}

void MainWindow::onPageChanged(int index) {
    qDebug() << "Page changed to index:" << index;
}
