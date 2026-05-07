#pragma once
#include <QMainWindow>
#include <QTabWidget>

class MonitoringClient;
class AdminClient;
class BIClient;
class TrajectoryClient;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow();

private slots:
    void onPageChanged(int index);

private:
    QTabWidget* tabWidget_;
    MonitoringClient* monitoringClient_;
    AdminClient* adminClient_;
    BIClient* biClient_;
    TrajectoryClient* trajectoryClient_;
};
