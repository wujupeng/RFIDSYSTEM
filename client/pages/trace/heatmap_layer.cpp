#include "heatmap_layer.h"
#include <QPainter>

HeatmapLayer::HeatmapLayer(QGraphicsItem* parent)
    : QGraphicsItem(parent), gridSize(50) {
}

void HeatmapLayer::updateCell(const QPoint& cell, int count) {
    heatGrid[QPair<int, int>(cell.x(), cell.y())] = count;
    update();
}

void HeatmapLayer::clear() {
    heatGrid.clear();
    update();
}

void HeatmapLayer::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    Q_UNUSED(option);
    Q_UNUSED(widget);
    
    for (auto it = heatGrid.begin(); it != heatGrid.end(); ++it) {
        QPair<int, int> cell = it.key();
        int count = it.value();
        
        QRectF rect(cell.first * gridSize, cell.second * gridSize, gridSize, gridSize);
        QColor color = getHeatColor(count);
        
        painter->fillRect(rect, color);
    }
}

QColor HeatmapLayer::getHeatColor(int count) {
    if (count > 20) {
        return QColor(255, 0, 0, 180);
    } else if (count > 10) {
        return QColor(255, 165, 0, 150);
    } else if (count > 5) {
        return QColor(255, 255, 0, 100);
    } else {
        return QColor(0, 255, 0, 60);
    }
}