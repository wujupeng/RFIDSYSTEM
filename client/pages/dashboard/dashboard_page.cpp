#include "dashboard_page.h"
#include <QFont>

DashboardPage::DashboardPage(QWidget* parent)
    : QWidget(parent) {

    auto* mainLayout = new QVBoxLayout(this);

    auto* titleLabel = new QLabel("Asset Management Dashboard");
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(18);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignCenter);

    QGridLayout* statsGrid = new QGridLayout();
    statsGrid->setSpacing(20);

    totalAssetsLabel_ = new QLabel("0");
    totalAssetsLabel_->setAlignment(Qt::AlignCenter);
    totalAssetsLabel_->setStyleSheet("font-size: 36px; font-weight: bold; color: #2c3e50;");

    inStockLabel_ = new QLabel("0");
    inStockLabel_->setAlignment(Qt::AlignCenter);
    inStockLabel_->setStyleSheet("font-size: 24px; color: #27ae60;");

    inUseLabel_ = new QLabel("0");
    inUseLabel_->setAlignment(Qt::AlignCenter);
    inUseLabel_->setStyleSheet("font-size: 24px; color: #3498db;");

    repairLabel_ = new QLabel("0");
    repairLabel_->setAlignment(Qt::AlignCenter);
    repairLabel_->setStyleSheet("font-size: 24px; color: #f39c12;");

    scrappedLabel_ = new QLabel("0");
    scrappedLabel_->setAlignment(Qt::AlignCenter);
    scrappedLabel_->setStyleSheet("font-size: 24px; color: #e74c3c;");

    statsGrid->addWidget(new QLabel("Total Assets"), 0, 0);
    statsGrid->addWidget(totalAssetsLabel_, 1, 0);
    statsGrid->addWidget(new QLabel("In Stock"), 0, 1);
    statsGrid->addWidget(inStockLabel_, 1, 1);
    statsGrid->addWidget(new QLabel("In Use"), 0, 2);
    statsGrid->addWidget(inUseLabel_, 1, 2);
    statsGrid->addWidget(new QLabel("Repair"), 0, 3);
    statsGrid->addWidget(repairLabel_, 1, 3);
    statsGrid->addWidget(new QLabel("Scrapped"), 0, 4);
    statsGrid->addWidget(scrappedLabel_, 1, 4);

    for (int col = 0; col < 5; ++col) {
        statsGrid->setColumnStretch(col, 1);
    }

    auto* refreshBtn = new QPushButton("Refresh Statistics");
    connect(refreshBtn, &QPushButton::clicked, this, &DashboardPage::refreshStats);

    mainLayout->addWidget(titleLabel);
    mainLayout->addSpacing(30);
    mainLayout->addLayout(statsGrid);
    mainLayout->addStretch();
    mainLayout->addWidget(refreshBtn);

    refreshStats();
}

void DashboardPage::refreshStats() {
}