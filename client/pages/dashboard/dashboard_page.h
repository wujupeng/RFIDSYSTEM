#pragma once
#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QPushButton>

class DashboardPage : public QWidget {
    Q_OBJECT

public:
    explicit DashboardPage(QWidget* parent = nullptr);

private slots:
    void refreshStats();

private:
    QLabel* totalAssetsLabel_;
    QLabel* inStockLabel_;
    QLabel* inUseLabel_;
    QLabel* repairLabel_;
    QLabel* scrappedLabel_;
};