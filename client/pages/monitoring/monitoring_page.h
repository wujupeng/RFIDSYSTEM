#pragma once
#include <QWidget>
#include <QLabel>
#include <QtCharts/QChartView>
#include <QtCharts/QPieSeries>
#include <QtCharts/QPieSlice>
#include <QtCharts/QLineSeries>
#include <QtCharts/QChart>

QT_USE_NAMESPACE

class MonitoringClient;

class MonitoringPage : public QWidget {
    Q_OBJECT

public:
    explicit MonitoringPage(MonitoringClient* client, QWidget* parent = nullptr);

private slots:
    void refresh();

private:
    MonitoringClient* client_;

    QLabel* statusLabel_;
    QLabel* latencyLabel_;
    QLabel* adoptionLabel_;

    QChartView* pieChartView_;
    QChartView* lineChartView_;
};
