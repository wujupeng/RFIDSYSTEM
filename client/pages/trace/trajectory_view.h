#pragma once

#include <QGraphicsView>
#include <QMap>
#include <vector>
#include <string>

#include "trajectory.grpc.pb.h"
#include "trajectory_scene.h"

class TrajectoryView : public QGraphicsView {
    Q_OBJECT
    
public:
    explicit TrajectoryView(QWidget* parent = nullptr);
    ~TrajectoryView();
    
    void setTrajectory(const std::vector<trajectory::TrajectoryPoint>& points);
    void playAnimation();
    void stopAnimation();
    void resetView();
    
    void setLocationMapping(const QMap<QString, QPointF>& mapping);
    void addLocation(const QString& name, const QPointF& position);
    
    void addAsset(int assetId, const QString& name, double risk = 0.0);
    void updateAsset(int assetId, const QString& location, double risk = 0.0, bool abnormal = false);
    void removeAsset(int assetId);
    
signals:
    void animationFinished();
    void pointReached(const trajectory::TrajectoryPoint& point);
    
private:
    void initLocationMapping();
    void drawLocations();
    
    TrajectoryScene* scene;
    QMap<QString, QPointF> locationMap;
    std::vector<trajectory::TrajectoryPoint> trajectoryPoints;
    int currentPointIndex;
    bool isPlaying;
};