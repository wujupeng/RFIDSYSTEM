#pragma once
#include <QWidget>
#include <QTableWidget>
#include <QPushButton>
#include <QLineEdit>

class AllocationPage : public QWidget {
    Q_OBJECT

public:
    explicit AllocationPage(QWidget* parent = nullptr);

private slots:
    void onApproveAllocation();
    void onReturnAsset();
    void onRefreshList();
    void showApplyDialog();

private:
    void setupUi();

    QTableWidget* allocationTable_;
    QLineEdit* searchEdit_;
};
