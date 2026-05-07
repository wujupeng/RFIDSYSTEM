#pragma once

#include <QGraphicsItem>
#include <QMap>
#include <QPointF>
#include <QVector>

class PathLayer : public QGraphicsItem {
public:
    PathLayer(QGraphicsItem* parent = nullptr);
    
    void addPath(int assetId, const QVector<QPointF>& points);
    void updatePath(int assetId, const QPointF& newPoint);
    void removePath(int assetId);
    void clear();
    
    QRectF boundingRect() const override { return QRectF(0, 0, 1000, 600); }
    
protected:
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    
private:
    QMap<int, QVector<QPointF>> paths;
};