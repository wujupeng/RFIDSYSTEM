#pragma once
#include <QWidget>
#include <QTableWidget>
#include <QPushButton>
#include <QLineEdit>

class InspectionPage : public QWidget {
    Q_OBJECT

public:
    explicit InspectionPage(QWidget* parent = nullptr);

private slots:
    void onStartInspection();
    void onScanAsset();
    void onCompleteRoute();
    void onRefreshList();

private:
    void setupUi();

    QTableWidget* inspectionTable_;
    QTableWidget* routeTable_;
    QLineEdit* searchEdit_;
};
