#include "trajectory_view.h"
#include <QGraphicsLineItem>
#include <QGraphicsTextItem>
#include <QGraphicsRectItem>
#include <QBrush>
#include <QPen>
#include <QVariantAnimation>
#include <QEasingCurve>

TrajectoryView::TrajectoryView(QWidget* parent) : QGraphicsView(parent) {
    scene = new TrajectoryScene(this);
    setScene(scene);
    setRenderHint(QPainter::Antialiasing);
    setBackgroundBrush(QColor(245, 245, 245));
    
    currentPointIndex = 0;
    isPlaying = false;
    
    initLocationMapping();
    drawLocations();
}

TrajectoryView::~TrajectoryView() {
    stopAnimation();
}

void TrajectoryView::initLocationMapping() {
    locationMap["机柜A"] = QPointF(100, 100);
    locationMap["通道"] = QPointF(300, 150);
    locationMap["机柜B"] = QPointF(500, 100);
    locationMap["仓库入口"] = QPointF(100, 300);
    locationMap["仓库出口"] = QPointF(500, 300);
    locationMap["检测区"] = QPointF(300, 300);
    locationMap["维修区"] = QPointF(200, 200);
    locationMap["存储区"] = QPointF(400, 200);
}

void TrajectoryView::drawLocations() {
    scene->clearAll();
    
    QPen linePen(Qt::gray, 2, Qt::DashLine);
    
    for (auto it = locationMap.begin(); it != locationMap.end(); ++it) {
        QPointF pos = it.value();
        
        QGraphicsRectItem* rect = new QGraphicsRectItem(pos.x() - 30, pos.y() - 20, 60, 40);
        rect->setBrush(QColor(200, 220, 240));
        rect->setPen(QPen(Qt::darkBlue, 1));
        scene->addItem(rect);
        
        QGraphicsTextItem* label = new QGraphicsTextItem(it.key());
        label->setPos(pos.x() - label->boundingRect().width() / 2, pos.y() - 35);
        label->setDefaultTextColor(Qt::darkBlue);
        scene->addItem(label);
    }
    
    QStringList connections = {"机柜A-通道", "通道-机柜B", "仓库入口-通道", "通道-检测区", 
                               "检测区-仓库出口", "维修区-通道", "存储区-通道"};
    
    for (const QString& conn : connections) {
        QStringList parts = conn.split("-");
        if (parts.size() == 2 && locationMap.contains(parts[0]) && locationMap.contains(parts[1])) {
            QPointF p1 = locationMap[parts[0]];
            QPointF p2 = locationMap[parts[1]];
            scene->addLine(QLineF(p1, p2), linePen);
        }
    }
    
    setSceneRect(-50, -50, 600, 400);
}

void TrajectoryView::setTrajectory(const std::vector<trajectory::TrajectoryPoint>& points) {
    stopAnimation();
    trajectoryPoints = points;
    currentPointIndex = 0;
    
    drawLocations();
}

void TrajectoryView::playAnimation() {
    if (trajectoryPoints.empty()) return;
    
    stopAnimation();
    isPlaying = true;
    currentPointIndex = 0;
    
    if (!trajectoryPoints.empty()) {
        QString firstLoc = QString::fromStdString(trajectoryPoints[0].location());
        if (locationMap.contains(firstLoc)) {
            scene->addAsset(1, "Asset-1", trajectoryPoints[0].risk_score());
            scene->updateAsset(1, locationMap[firstLoc], trajectoryPoints[0].risk_score(), false);
        }
        
        for (size_t i = 0; i < trajectoryPoints.size() - 1; ++i) {
            QString fromLoc = QString::fromStdString(trajectoryPoints[i].location());
            QString toLoc = QString::fromStdString(trajectoryPoints[i+1].location());
            
            if (locationMap.contains(fromLoc) && locationMap.contains(toLoc)) {
                QPointF from = locationMap[fromLoc];
                QPointF to = locationMap[toLoc];
                
                QVariantAnimation* anim = new QVariantAnimation(this);
                anim->setDuration(1500);
                anim->setStartValue(from);
                anim->setEndValue(to);
                anim->setEasingCurve(QEasingCurve::InOutQuad);
                
                connect(anim, &QVariantAnimation::valueChanged, [this](const QVariant& value) {
                    scene->updateAsset(1, value.toPointF(), trajectoryPoints[currentPointIndex].risk_score(), false);
                });
                
                connect(anim, &QVariantAnimation::finished, [this, i]() {
                    currentPointIndex = static_cast<int>(i) + 1;
                    if (currentPointIndex >= static_cast<int>(trajectoryPoints.size()) - 1) {
                        isPlaying = false;
                        emit animationFinished();
                    }
                });
                
                QTimer::singleShot(i * 1600, [anim]() {
                    anim->start();
                });
            }
        }
    }
}

void TrajectoryView::stopAnimation() {
    isPlaying = false;
    currentPointIndex = 0;
    
    QList<QVariantAnimation*> animations = findChildren<QVariantAnimation*>();
    for (QVariantAnimation* anim : animations) {
        anim->stop();
        anim->deleteLater();
    }
}

void TrajectoryView::resetView() {
    stopAnimation();
    trajectoryPoints.clear();
    currentPointIndex = 0;
    drawLocations();
}

void TrajectoryView::setLocationMapping(const QMap<QString, QPointF>& mapping) {
    locationMap = mapping;
    drawLocations();
}

void TrajectoryView::addLocation(const QString& name, const QPointF& position) {
    locationMap[name] = position;
    drawLocations();
}

void TrajectoryView::addAsset(int assetId, const QString& name, double risk) {
    scene->addAsset(assetId, name, risk);
}

void TrajectoryView::updateAsset(int assetId, const QString& location, double risk, bool abnormal) {
    if (locationMap.contains(location)) {
        scene->updateAsset(assetId, locationMap[location], risk, abnormal);
    }
}

void TrajectoryView::removeAsset(int assetId) {
    scene->removeAsset(assetId);
}