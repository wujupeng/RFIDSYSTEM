#pragma once
#include <QWidget>
#include <QTableWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QGroupBox>
#include <QTimer>
#include <QMutex>

class InventoryScanPage : public QWidget {
    Q_OBJECT

public:
    explicit InventoryScanPage(QWidget* parent = nullptr);
    ~InventoryScanPage();

signals:
    void scanResultsReady(const QStringList& epcs, const QString& status);

private slots:
    void onStartScan();
    void onStopScan();
    void onAddEPC();
    void onClearAll();
    void onBatchScan();
    void onRefreshTimer();

private:
    void updateStats();
    void addScanResult(const QString& epc, const QString& status, const QString& assetInfo = "");
    void queueResult(const QString& epc, const QString& status, const QString& assetInfo);

    QLineEdit* epcInput_;
    QPushButton* startBtn_;
    QPushButton* stopBtn_;
    QPushButton* addBtn_;
    QPushButton* clearBtn_;
    QPushButton* batchScanBtn_;

    QTableWidget* scanTable_;
    
    QLabel* scannedCountLabel_;
    QLabel* foundCountLabel_;
    QLabel* missingCountLabel_;
    QLabel* extraCountLabel_;

    bool isScanning_;
    int taskId_;

    QTimer* refreshTimer_;
    bool pendingRefresh_;
    QMutex resultMutex_;
    struct PendingResult {
        QString epc;
        QString status;
        QString assetInfo;
    };
    QList<PendingResult> pendingResults_;
};