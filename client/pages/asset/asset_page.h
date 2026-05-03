#pragma once
#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QLabel>

class AssetPage : public QWidget {
    Q_OBJECT

public:
    explicit AssetPage(QWidget* parent = nullptr);

private slots:
    void onCreateAsset();
    void onUpdateStatus();
    void onRefreshList();

private:
    void setupUi();
    void clearForm();

    QLineEdit* nameInput_;
    QLineEdit* typeInput_;
    QLineEdit* assetCodeInput_;
    QLineEdit* rfidEpcInput_;
    QLineEdit* locationInput_;
    QLineEdit* operatorInput_;

    QTableWidget* assetTable_;
    QLabel* statusLabel_;
};