#pragma once

#include <QObject>
#include <QMutex>
#include <QRectF>
#include <QVector>
#include <memory>
#include <functional>

struct SpatialEntry {
    int id;
    double x;
    double y;
    double radius;
    std::string type;
    void* data;
};

class SpatialIndex : public QObject {
    Q_OBJECT
    
public:
    SpatialIndex(QObject* parent = nullptr);
    ~SpatialIndex();
    
    void initialize(double width, double height, int gridSize = 100);
    
    void insert(int id, double x, double y, double radius = 0.0, 
                const std::string& type = "asset", void* data = nullptr);
    
    void update(int id, double x, double y);
    
    void remove(int id);
    
    QVector<SpatialEntry> query(const QRectF& area) const;
    
    QVector<SpatialEntry> queryRadius(double x, double y, double radius) const;
    
    QVector<SpatialEntry> getNearest(double x, double y, int count) const;
    
    int countInArea(const QRectF& area) const;
    
    int totalCount() const;
    
    void clear();
    
    void setGridSize(int size);
    int gridSize() const;
    
signals:
    void indexUpdated();
    
private:
    struct GridCell {
        QVector<SpatialEntry> entries;
    };
    
    int getGridX(double x) const;
    int getGridY(double y) const;
    
    mutable QMutex mutex_;
    double width_;
    double height_;
    int gridSize_;
    int gridCols_;
    int gridRows_;
    
    std::vector<std::vector<GridCell>> grid_;
    QMap<int, SpatialEntry> entries_;
};