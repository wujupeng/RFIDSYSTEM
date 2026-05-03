#pragma once
#include <QMainWindow>
#include <QStackedWidget>

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow();

private slots:
    void onPageChanged(int index);

private:
    QStackedWidget* stackedWidget_;
};