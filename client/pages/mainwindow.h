#pragma once
#include <QMainWindow>
#include <QTabWidget>

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow();

private slots:
    void onPageChanged(int index);

private:
    QTabWidget* tabWidget_;
};
