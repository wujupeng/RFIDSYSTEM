#pragma once
#include <QWidget>
#include <QTableWidget>
#include <QPushButton>
#include <QLabel>

class RepairPage : public QWidget {
    Q_OBJECT

public:
    explicit RepairPage(QWidget* parent = nullptr);

private slots:
    void onSubmitRepair();
    void onRefreshRepairList();
    void onCompleteRepair();

private:
    void setupUi();

    QTableWidget* repairTable_;
    QLabel* statusLabel_;
};