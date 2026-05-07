#pragma once

#include <QGraphicsEllipseItem>
#include <QVector>
#include <QPointF>
#include <QString>

class AssetItem : public QGraphicsEllipseItem {
public:
    AssetItem(int assetId, const QString& name, QGraphicsItem* parent = nullptr);
    
    int assetId() const { return m_assetId; }
    QString assetName() const { return m_assetName; }
    
    double riskScore() const { return m_riskScore; }
    void setRiskScore(double score);
    
    QString currentLocation() const { return m_currentLocation; }
    void setCurrentLocation(const QString& location) { m_currentLocation = location; }
    
    void updatePosition(const QPointF& pos);
    void addTrailPoint(const QPointF& p);
    void clearTrail();
    
    void setAbnormal(bool abnormal) { m_abnormal = abnormal; }
    bool isAbnormal() const { return m_abnormal; }
    
    void update(double time);
    
protected:
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    
private:
    void updateRiskColor();
    void drawTrail(QPainter* painter);
    void drawBreathingEffect(QPainter* painter);
    void drawPulseEffect(QPainter* painter);
    
    int m_assetId;
    QString m_assetName;
    double m_riskScore;
    QString m_currentLocation;
    
    QVector<QPointF> m_trailPoints;
    static constexpr int MAX_TRAIL_POINTS = 50;
    
    bool m_abnormal;
    double m_timeOffset;
};