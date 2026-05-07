#pragma once

#include <QGraphicsItem>
#include <QMap>
#include <QPointF>

struct AlertInfo {
    QPointF position;
    double startTime;
    QString message;
    int severity; // 0: info, 1: warning, 2: critical
};

class AlertLayer : public QGraphicsItem {
public:
    AlertLayer(QGraphicsItem* parent = nullptr);
    
    void addAlert(int alertId, const QPointF& pos, const QString& message, int severity = 1);
    void removeAlert(int alertId);
    void clear();
    void update(double currentTime);
    
    QRectF boundingRect() const override { return QRectF(0, 0, 1000, 600); }
    
protected:
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    
private:
    QColor getAlertColor(int severity);
    
    QMap<int, AlertInfo> alerts;
};