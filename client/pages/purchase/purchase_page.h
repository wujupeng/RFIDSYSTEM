#pragma once
#include <QWidget>
#include <QTableWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>

class PurchasePage : public QWidget {
    Q_OBJECT

public:
    explicit PurchasePage(QWidget* parent = nullptr);

private slots:
    void onAddPurchase();
    void onRFIDWrite();
    void onRefreshList();
    void onCompletePurchase();
    void showPurchaseDialog();

private:
    void setupUi();
    QString generateAssetCode();

    QTableWidget* purchaseTable_;
    QLineEdit* searchEdit_;
};
