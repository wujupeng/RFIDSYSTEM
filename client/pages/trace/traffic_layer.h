#pragma once

#include <QGraphicsItem>
#include <QMap>
#include <QPoint>
#include <QPair>

struct CongestionCell {
    int deviceCount;
    int movementCount;
    double congestionScore;
};

class TrafficLayer : public QGraphicsItem {
public:
    TrafficLayer(QGraphicsItem* parent = nullptr);
    
    void setGridSize(int size) { gridSize = size; }
    void updateCell(const QPoint& cell, int deviceCount, int movementCount);
    void clear();
    void resetMovement();
    
    QRectF boundingRect() const override { return QRectF(0, 0, 1000, 600); }
    
protected:
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    
private:
    QColor getCongestionColor(double score);
    
    int gridSize;
    QMap<QPair<int, int>, CongestionCell> congestionGrid;
};