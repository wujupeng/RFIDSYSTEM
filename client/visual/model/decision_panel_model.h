#pragma once

#include <QObject>
#include <QString>
#include <QVariantList>
#include <QVariantMap>
#include "../../../server/runtime/spatial_frame.h"

class DecisionPanelModel : public QObject {
    Q_OBJECT
    
    Q_PROPERTY(QString assetName READ assetName NOTIFY assetNameChanged)
    Q_PROPERTY(QString action READ action NOTIFY actionChanged)
    Q_PROPERTY(double confidence READ confidence NOTIFY confidenceChanged)
    Q_PROPERTY(double missingRisk READ missingRisk NOTIFY missingRiskChanged)
    Q_PROPERTY(double abnormalRisk READ abnormalRisk NOTIFY abnormalRiskChanged)
    Q_PROPERTY(double idleRisk READ idleRisk NOTIFY idleRiskChanged)
    Q_PROPERTY(QString reason READ reason NOTIFY reasonChanged)
    Q_PROPERTY(QVariantList topKActions READ topKActions NOTIFY topKActionsChanged)
    Q_PROPERTY(bool visible READ visible NOTIFY visibleChanged)
    
public:
    explicit DecisionPanelModel(QObject* parent = nullptr);
    
    QString assetName() const;
    QString action() const;
    double confidence() const;
    double missingRisk() const;
    double abnormalRisk() const;
    double idleRisk() const;
    QString reason() const;
    QVariantList topKActions() const;
    bool visible() const;
    
    Q_INVOKABLE void show(uint64_t assetId, double x, double y);
    Q_INVOKABLE void hide();
    Q_INVOKABLE void reportFeedback(const QString& action);
    
    void updateOverlayData(const DecisionOverlay& overlay);
    void setTopKActions(const std::vector<std::pair<std::string, double>>& actions);
    
signals:
    void assetNameChanged();
    void actionChanged();
    void confidenceChanged();
    void missingRiskChanged();
    void abnormalRiskChanged();
    void idleRiskChanged();
    void reasonChanged();
    void topKActionsChanged();
    void visibleChanged();
    void feedbackReported(const QString& assetName, const QString& action);
    
private:
    QString asset_name_;
    QString action_;
    double confidence_;
    double missing_risk_;
    double abnormal_risk_;
    double idle_risk_;
    QString reason_;
    QVariantList top_k_actions_;
    bool visible_;
    uint64_t current_asset_id_;
    
    QString actionToString(uint32_t action) const;
};