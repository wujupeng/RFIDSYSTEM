#include "trajectory_scene.h"
#include "asset_item.h"
#include "heatmap_layer.h"
#include "traffic_layer.h"
#include "path_layer.h"
#include "alert_layer.h"
#include <QPair>

TrajectoryScene::TrajectoryScene(QObject* parent)
    : QGraphicsScene(parent), gridSize(50), time(0.0) {
    
    heatmapLayer = new HeatmapLayer();
    heatmapLayer->setGridSize(gridSize);
    addItem(heatmapLayer);
    
    trafficLayer = new TrafficLayer();
    trafficLayer->setGridSize(gridSize);
    addItem(trafficLayer);
    
    pathLayer = new PathLayer();
    addItem(pathLayer);
    
    alertLayer = new AlertLayer();
    addItem(alertLayer);
    
    tickTimer = new QTimer(this);
    connect(tickTimer, &QTimer::timeout, this, &TrajectoryScene::onTick);
    tickTimer->start(33);
}

TrajectoryScene::~TrajectoryScene() {
    tickTimer->stop();
    delete tickTimer;
    
    qDeleteAll(assets);
    assets.clear();
}

void TrajectoryScene::addAsset(int assetId, const QString& name, double risk) {
    if (assets.contains(assetId)) {
        return;
    }
    
    AssetItem* item = new AssetItem(assetId, name);
    item->setRiskScore(risk);
    assets[assetId] = item;
    addItem(item);
}

void TrajectoryScene::updateAsset(int assetId, const QPointF& pos, double risk, bool abnormal) {
    if (!assets.contains(assetId)) {
        return;
    }
    
    AssetItem* item = assets[assetId];
    item->updatePosition(pos);
    item->setRiskScore(risk);
    item->setAbnormal(abnormal);
    item->update(time);
    
    pathLayer->updatePath(assetId, pos);
}

void TrajectoryScene::removeAsset(int assetId) {
    if (!assets.contains(assetId)) {
        return;
    }
    
    AssetItem* item = assets.take(assetId);
    removeItem(item);
    delete item;
    
    pathLayer->removePath(assetId);
}

void TrajectoryScene::clearAll() {
    qDeleteAll(assets);
    assets.clear();
    
    heatmapLayer->clear();
    trafficLayer->clear();
    pathLayer->clear();
    alertLayer->clear();
}

void TrajectoryScene::onTick() {
    time += 0.033;
    
    QMap<QPair<int, int>, int> deviceCounts;
    
    for (auto it = assets.begin(); it != assets.end(); ++it) {
        AssetItem* item = it.value();
        item->update(time);
        
        int cellX = static_cast<int>(item->pos().x() / gridSize);
        int cellY = static_cast<int>(item->pos().y() / gridSize);
        deviceCounts[QPair<int, int>(cellX, cellY)]++;
    }
    
    for (auto it = deviceCounts.begin(); it != deviceCounts.end(); ++it) {
        QPair<int, int> key = it.key();
        QPoint cell(key.first, key.second);
        heatmapLayer->updateCell(cell, it.value());
        trafficLayer->updateCell(cell, it.value(), 1);
    }
    
    alertLayer->update(time);
    
    emit tick();
}