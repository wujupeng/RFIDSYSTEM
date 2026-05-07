#include "mainwindow.h"
#include "dashboard/dashboard_page.h"
#include "asset/asset_page.h"
#include "purchase/purchase_page.h"
#include "allocation/allocation_page.h"
#include "inventory/inventory_page.h"
#include "repair/repair_page.h"
#include "inspection/inspection_page.h"
#include "recommendations/system_recommendations_page.h"
#include "monitoring/monitoring_page.h"
#include "admin/user_management_page.h"
#include "admin/role_management_page.h"
#include "admin/system_settings_page.h"
#include "bi/bi_report_page.h"
#include "trace/trace_page.h"
#include "../network/monitoring_client.h"
#include "../network/admin_client.h"
#include "../network/bi_client.h"
#include "../network/trajectory_client.h"

#include <grpc/grpc.h>
#include <grpcpp/grpcpp.h>

MainWindow::MainWindow() : monitoringClient_(nullptr), adminClient_(nullptr), biClient_(nullptr), trajectoryClient_(nullptr) {
    setWindowTitle("RFID资产管理系统 v3.3.1 - 管理后台");
    resize(1400, 900);

    auto channel = grpc::CreateChannel("localhost:50051",
                                       grpc::InsecureChannelCredentials());
    monitoringClient_ = new MonitoringClient(channel);
    adminClient_ = new AdminClient(channel);
    biClient_ = new BIClient(channel);
    trajectoryClient_ = new TrajectoryClient(channel);

    tabWidget_ = new QTabWidget(this);
    tabWidget_->setTabPosition(QTabWidget::North);
    tabWidget_->setStyleSheet(R"(
        QTabWidget::pane { border: none; }
        QTabBar::tab {
            background: #e0e0e0;
            padding: 10px 16px;
            border: 1px solid #bdbdbd;
            border-bottom: none;
            border-top-left-radius: 5px;
            border-top-right-radius: 5px;
            font-size: 13px;
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
    tabWidget_->addTab(new PurchasePage(), "📦 采购入库");
    tabWidget_->addTab(new AllocationPage(), "📤 领用出库");
    tabWidget_->addTab(new InventoryPage(), "🔍 RFID盘点");
    tabWidget_->addTab(new RepairPage(), "🔧 维修管理");
    tabWidget_->addTab(new InspectionPage(), "📋 定期巡检");
    tabWidget_->addTab(new SystemRecommendationsPage(), "⚠️ 系统建议");
    tabWidget_->addTab(new MonitoringPage(monitoringClient_), "🔬 运维控制台");
    tabWidget_->addTab(new BIReportPage(biClient_), "📈 BI报表");
    tabWidget_->addTab(new TracePage(trajectoryClient_), "📍 轨迹追踪");
    tabWidget_->addTab(new UserManagementPage(adminClient_), "👥 用户管理");
    tabWidget_->addTab(new RoleManagementPage(adminClient_), "🎭 角色管理");
    tabWidget_->addTab(new SystemSettingsPage(adminClient_), "⚙️ 系统设置");

    setCentralWidget(tabWidget_);

    connect(tabWidget_, &QTabWidget::currentChanged, this, &MainWindow::onPageChanged);
}

void MainWindow::onPageChanged(int index) {
    qDebug() << "Page changed to index:" << index;
}
