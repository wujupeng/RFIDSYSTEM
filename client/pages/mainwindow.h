#pragma once
#include <QMainWindow>
#include <QTabWidget>

class MonitoringClient;
class AdminClient;

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
};
