#include "asset_item.h"
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QDateTime>

AssetItem::AssetItem(int assetId, const QString& name, QGraphicsItem* parent)
    : QGraphicsEllipseItem(-10, -10, 20, 20, parent),
      m_assetId(assetId),
      m_assetName(name),
      m_riskScore(0.0),
      m_abnormal(false),
      m_timeOffset(rand() % 1000 / 1000.0) {
    
    setBrush(Qt::green);
    setPen(QPen(Qt::white, 2));
    setZValue(100);
    setCacheMode(QGraphicsItem::DeviceCoordinateCache);
}

void AssetItem::setRiskScore(double score) {
    m_riskScore = score;
    updateRiskColor();
}

void AssetItem::updatePosition(const QPointF& pos) {
    addTrailPoint(pos);
    setPos(pos);
}

void AssetItem::addTrailPoint(const QPointF& p) {
    m_trailPoints.append(p);
    if (m_trailPoints.size() > MAX_TRAIL_POINTS) {
        m_trailPoints.removeFirst();
    }
}

void AssetItem::clearTrail() {
    m_trailPoints.clear();
}

void AssetItem::update(double time) {
    if (m_abnormal || m_riskScore > 0.8) {
        double opacity = 0.5 + 0.5 * sin(time * 5 + m_timeOffset * 6.28);
        setOpacity(opacity);
    } else {
        setOpacity(1.0);
    }
}

void AssetItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    Q_UNUSED(option);
    Q_UNUSED(widget);
    
    drawTrail(painter);
    
    if (m_riskScore > 0.8) {
        drawPulseEffect(painter);
    }
    
    drawBreathingEffect(painter);
}

void AssetItem::updateRiskColor() {
    if (m_riskScore > 0.8) {
        setBrush(Qt::red);
    } else if (m_riskScore > 0.5) {
        setBrush(Qt::yellow);
    } else {
        setBrush(Qt::green);
    }
}

void AssetItem::drawTrail(QPainter* painter) {
    if (m_trailPoints.size() < 2) return;
    
    QPen trailPen(Qt::cyan, 2);
    painter->setPen(trailPen);
    
    int total = m_trailPoints.size();
    for (int i = 1; i < total; ++i) {
        double alpha = static_cast<double>(i) / total;
        QColor color(0, 255, 255, static_cast<int>(255 * alpha));
        painter->setPen(QPen(color, 2));
        painter->drawLine(m_trailPoints[i-1], m_trailPoints[i]);
    }
}

void AssetItem::drawBreathingEffect(QPainter* painter) {
    double scale = 1.0 + 0.1 * sin(QDateTime::currentMSecsSinceEpoch() / 500.0 + m_timeOffset * 6.28);
    painter->save();
    painter->translate(boundingRect().center());
    painter->scale(scale, scale);
    painter->translate(-boundingRect().center());
    
    QGraphicsEllipseItem::paint(painter, nullptr, nullptr);
    
    painter->restore();
}

void AssetItem::drawPulseEffect(QPainter* painter) {
    double pulseScale = 1.5 + 0.5 * sin(QDateTime::currentMSecsSinceEpoch() / 300.0);
    QColor pulseColor(255, 0, 0, 50);
    
    painter->save();
    painter->translate(boundingRect().center());
    painter->scale(pulseScale, pulseScale);
    
    painter->setBrush(pulseColor);
    painter->setPen(Qt::NoPen);
    painter->drawEllipse(boundingRect());
    
    painter->restore();
}