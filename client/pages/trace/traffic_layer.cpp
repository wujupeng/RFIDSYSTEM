#include "traffic_layer.h"
#include <QPainter>

TrafficLayer::TrafficLayer(QGraphicsItem* parent)
    : QGraphicsItem(parent), gridSize(50) {
}

void TrafficLayer::updateCell(const QPoint& cell, int deviceCount, int movementCount) {
    QPair<int, int> key(cell.x(), cell.y());
    CongestionCell& c = congestionGrid[key];
    c.deviceCount = deviceCount;
    c.movementCount = movementCount;
    
    double deviceScore = std::min(static_cast<double>(deviceCount) / 30.0, 1.0);
    double movementScore = std::min(static_cast<double>(movementCount) / 10.0, 1.0);
    
    c.congestionScore = (deviceScore * 0.6 + movementScore * 0.4) * 100;
    
    update();
}

void TrafficLayer::clear() {
    congestionGrid.clear();
    update();
}

void TrafficLayer::resetMovement() {
    for (auto it = congestionGrid.begin(); it != congestionGrid.end(); ++it) {
        it.value().movementCount = 0;
    }
    update();
}

void TrafficLayer::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    Q_UNUSED(option);
    Q_UNUSED(widget);
    
    for (auto it = congestionGrid.begin(); it != congestionGrid.end(); ++it) {
        QPair<int, int> cell = it.key();
        const CongestionCell& c = it.value();
        
        QRectF rect(cell.first * gridSize, cell.second * gridSize, gridSize, gridSize);
        QColor color = getCongestionColor(c.congestionScore);
        
        painter->fillRect(rect, color);
        
        if (c.congestionScore > 30) {
            QString label = QString("%1%").arg(static_cast<int>(c.congestionScore));
            painter->drawText(rect, Qt::AlignCenter, label);
        }
    }
}

QColor TrafficLayer::getCongestionColor(double score) {
    if (score >= 90) {
        return QColor(255, 0, 0, 150);
    } else if (score >= 60) {
        return QColor(255, 140, 0, 120);
    } else if (score >= 30) {
        return QColor(255, 215, 0, 100);
    } else {
        return QColor(0, 255, 0, 60);
    }
}