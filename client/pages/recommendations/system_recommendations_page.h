#pragma once
#include <QWidget>
#include <QTableWidget>
#include <QLabel>
#include <QTimer>
#include <map>

class GrpcClient;

class SystemRecommendationsPage : public QWidget {
    Q_OBJECT

public:
    explicit SystemRecommendationsPage(GrpcClient* grpcClient = nullptr, QWidget* parent = nullptr);
    ~SystemRecommendationsPage() = default;

private slots:
    void refreshRecommendations();
    void onConfirmClicked(int row);
    void onIgnoreClicked(int row);

private:
    void setupUI();
    void loadRecommendations();
    void loadMockData();
    void deduplicateRecommendations();
    QString getRiskLevelString(int priority);
    QString getActionString(const QString& action);
    QColor getRiskColor(int priority);
    void applyRowStyle(int row, int priority);
    void updateStats();

    struct RecommendationItem {
        int asset_id;
        QString asset_name;
        QString location;
        QString action;
        int priority;
        QString reason;
        QString timestamp;
        bool is_handled;
        bool is_executed;
        bool is_ignored;
    };

    QTableWidget* table_;
    QLabel* totalLabel_;
    QLabel* highPriorityLabel_;
    QLabel* mediumPriorityLabel_;
    QLabel* lowPriorityLabel_;
    QLabel* adoptionRateLabel_;

    QTimer* refreshTimer_;
    std::vector<RecommendationItem> recommendations_;
    QString current_user_;
    GrpcClient* grpcClient_;
    double cachedAdoptionRate_;
};
