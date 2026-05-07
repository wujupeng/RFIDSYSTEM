#pragma once

#include <QGraphicsScene>
#include <QTimer>
#include <QMap>
#include <QVector>
#include <QPoint>

class AssetItem;
class HeatmapLayer;
class TrafficLayer;
class PathLayer;
class AlertLayer;

class TrajectoryScene : public QGraphicsScene {
    Q_OBJECT
    
public:
    explicit TrajectoryScene(QObject* parent = nullptr);
    ~TrajectoryScene();
    
    void addAsset(int assetId, const QString& name, double risk = 0.0);
    void updateAsset(int assetId, const QPointF& pos, double risk = 0.0, bool abnormal = false);
    void removeAsset(int assetId);
    void clearAll();
    
    void setGridSize(int size) { gridSize = size; }
    
signals:
    void tick();
    
private slots:
    void onTick();
    
private:
    QTimer* tickTimer;
    int gridSize;
    
    HeatmapLayer* heatmapLayer;
    TrafficLayer* trafficLayer;
    PathLayer* pathLayer;
    AlertLayer* alertLayer;
    
    QMap<int, AssetItem*> assets;
    double time;
    
    void updateHeatmap();
    void updateTraffic();
};