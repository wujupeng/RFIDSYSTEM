#include "path_layer.h"
#include <QPainter>

PathLayer::PathLayer(QGraphicsItem* parent)
    : QGraphicsItem(parent) {
}

void PathLayer::addPath(int assetId, const QVector<QPointF>& points) {
    paths[assetId] = points;
    update();
}

void PathLayer::updatePath(int assetId, const QPointF& newPoint) {
    if (paths.contains(assetId)) {
        paths[assetId].append(newPoint);
        if (paths[assetId].size() > 100) {
            paths[assetId].removeFirst();
        }
    } else {
        QVector<QPointF> points;
        points.append(newPoint);
        paths[assetId] = points;
    }
    update();
}

void PathLayer::removePath(int assetId) {
    paths.remove(assetId);
    update();
}

void PathLayer::clear() {
    paths.clear();
    update();
}

void PathLayer::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    Q_UNUSED(option);
    Q_UNUSED(widget);
    
    QPen pathPen(Qt::darkGray, 2, Qt::DashLine);
    painter->setPen(pathPen);
    
    for (auto it = paths.begin(); it != paths.end(); ++it) {
        const QVector<QPointF>& points = it.value();
        if (points.size() < 2) continue;
        
        for (int i = 1; i < points.size(); ++i) {
            painter->drawLine(points[i-1], points[i]);
        }
    }
}