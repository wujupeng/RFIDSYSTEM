#include "monitoring_page.h"
#include "network/monitoring_client.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTimer>
#include <QGroupBox>
#include <QFormLayout>
#include <QPushButton>

MonitoringPage::MonitoringPage(MonitoringClient* client, QWidget* parent)
    : QWidget(parent), client_(client)
{
    auto* mainLayout = new QVBoxLayout(this);

    QGroupBox* statusGroup = new QGroupBox("System Health");
    QFormLayout* statusLayout = new QFormLayout();

    statusLabel_ = new QLabel("Checking...");
    statusLabel_->setStyleSheet("font-size: 14px; font-weight: bold;");
    latencyLabel_ = new QLabel("Latency: -- ms");
    latencyLabel_->setStyleSheet("font-size: 14px;");

    statusLayout->addRow("Status:", statusLabel_);
    statusLayout->addRow("Latency:", latencyLabel_);
    statusGroup->setLayout(statusLayout);
    mainLayout->addWidget(statusGroup);

    QGroupBox* pieGroup = new QGroupBox("Decision Distribution");
    pieChartView_ = new QChartView();
    pieChartView_->setMinimumHeight(250);
    auto* pieLayout = new QVBoxLayout();
    pieLayout->addWidget(pieChartView_);
    pieGroup->setLayout(pieLayout);
    mainLayout->addWidget(pieGroup);

    QGroupBox* adoptionGroup = new QGroupBox("User Adoption");
    adoptionLabel_ = new QLabel("Adoption Rate: --%");
    adoptionLabel_->setStyleSheet("font-size: 16px; font-weight: bold; color: green;");
    lineChartView_ = new QChartView();
    lineChartView_->setMinimumHeight(200);
    auto* adoptionLayout = new QVBoxLayout();
    adoptionLayout->addWidget(adoptionLabel_);
    adoptionLayout->addWidget(lineChartView_);
    adoptionGroup->setLayout(adoptionLayout);
    mainLayout->addWidget(adoptionGroup);

    auto* refreshBtn = new QPushButton("Refresh Now");
    connect(refreshBtn, &QPushButton::clicked, this, &MonitoringPage::refresh);
    mainLayout->addWidget(refreshBtn);

    auto* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MonitoringPage::refresh);
    timer->start(3000);

    refresh();
}

void MonitoringPage::refresh()
{
    auto health = client_->getHealth();

    QString status = QString("Server: %1 | gRPC: %2 | Bandit: %3")
        .arg(health.server_ok() ? "OK" : "ERR")
        .arg(health.grpc_ok() ? "OK" : "ERR")
        .arg(health.bandit_ok() ? "OK" : "WARN");

    statusLabel_->setText(status);
    statusLabel_->setStyleSheet(QString("font-size: 14px; font-weight: bold; color: %1;")
        .arg(health.server_ok() ? "green" : "red"));
    latencyLabel_->setText(QString("Latency: %1 ms").arg(health.latency_ms()));

    auto dist = client_->getDistribution();

    QPieSeries* series = new QPieSeries();
    series->append(QString("INSPECT (%1)").arg(dist.inspect()), dist.inspect());
    series->append(QString("NO_ACTION (%1)").arg(dist.no_action()), dist.no_action());
    series->append(QString("ALERT (%1)").arg(dist.alert()), dist.alert());

    QChart* pieChart = new QChart();
    pieChart->addSeries(series);
    pieChart->setTitle("Action Distribution");
    pieChart->legend()->setVisible(true);

    pieChartView_->setChart(pieChart);

    auto adoption = client_->getAdoption();

    adoptionLabel_->setText(
        QString("Adoption Rate: %1%")
            .arg(adoption.adoption_rate() * 100, 0, 'f', 1)
    );
    adoptionLabel_->setStyleSheet(QString("font-size: 16px; font-weight: bold; color: %1;")
        .arg(adoption.adoption_rate() >= 0.6 ? "green" : (adoption.adoption_rate() >= 0.4 ? "orange" : "red")));

    QLineSeries* line = new QLineSeries();
    int i = 0;
    for (auto v : adoption.history()) {
        line->append(i++, v);
    }

    QChart* lineChart = new QChart();
    lineChart->addSeries(line);
    lineChart->createDefaultAxes();
    lineChart->setTitle("Adoption Trend");
    lineChart->legend()->setVisible(false);

    lineChartView_->setChart(lineChart);
}
