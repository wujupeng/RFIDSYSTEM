#pragma once
#include <QWidget>
#include <QTableWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>

class InventoryPage : public QWidget {
    Q_OBJECT

public:
    explicit InventoryPage(QWidget* parent = nullptr);

private slots:
    void onSearch();
    void onExport();
    void onFilterChanged(int index);

private:
    void setupUi();

    QLineEdit* searchInput_;
    QComboBox* statusFilter_;
    QTableWidget* inventoryTable_;
    QLabel* totalLabel_;
};