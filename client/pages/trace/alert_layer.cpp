#include "alert_layer.h"
#include <QPainter>
#include <QDateTime>

AlertLayer::AlertLayer(QGraphicsItem* parent)
    : QGraphicsItem(parent) {
}

void AlertLayer::addAlert(int alertId, const QPointF& pos, const QString& message, int severity) {
    AlertInfo info;
    info.position = pos;
    info.startTime = QDateTime::currentMSecsSinceEpoch() / 1000.0;
    info.message = message;
    info.severity = severity;
    
    alerts[alertId] = info;
    QGraphicsItem::update();
}

void AlertLayer::removeAlert(int alertId) {
    alerts.remove(alertId);
    QGraphicsItem::update();
}

void AlertLayer::clear() {
    alerts.clear();
    QGraphicsItem::update();
}

void AlertLayer::update(double currentTime) {
    Q_UNUSED(currentTime);
    QGraphicsItem::update();
}

void AlertLayer::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    Q_UNUSED(option);
    Q_UNUSED(widget);
    
    double currentTime = QDateTime::currentMSecsSinceEpoch() / 1000.0;
    
    for (auto it = alerts.begin(); it != alerts.end(); ++it) {
        const AlertInfo& info = it.value();
        double elapsed = currentTime - info.startTime;
        
        double pulseScale = 1.0 + 0.3 * sin(elapsed * 3);
        QColor color = getAlertColor(info.severity);
        
        painter->save();
        painter->translate(info.position);
        painter->scale(pulseScale, pulseScale);
        
        painter->setBrush(QColor(color.red(), color.green(), color.blue(), 100));
        painter->setPen(QPen(color, 2));
        painter->drawEllipse(-20, -20, 40, 40);
        
        painter->restore();
        
        painter->setPen(color);
        painter->drawText(info.position.x() - 30, info.position.y() - 30, 60, 20, 
                         Qt::AlignCenter, info.message);
    }
}

QColor AlertLayer::getAlertColor(int severity) {
    switch (severity) {
        case 0:
            return QColor(0, 191, 255);
        case 2:
            return QColor(255, 0, 0);
        default:
            return QColor(255, 165, 0);
    }
}