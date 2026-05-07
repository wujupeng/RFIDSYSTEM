#pragma once

#include <QGraphicsItem>
#include <QMap>
#include <QPoint>
#include <QRectF>
#include <QPair>

class HeatmapLayer : public QGraphicsItem {
public:
    HeatmapLayer(QGraphicsItem* parent = nullptr);
    
    void setGridSize(int size) { gridSize = size; }
    void updateCell(const QPoint& cell, int count);
    void clear();
    
    QRectF boundingRect() const override { return QRectF(0, 0, 1000, 600); }
    
protected:
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    
private:
    QColor getHeatColor(int count);
    
    int gridSize;
    QMap<QPair<int, int>, int> heatGrid;
};