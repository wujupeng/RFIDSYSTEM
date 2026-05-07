#pragma once

#include <QObject>
#include <QAbstractListModel>
#include <QVector>
#include <QMap>

struct TrajectoryPoint {
    float x;
    float y;
    double timestamp;
    float alpha;
};

struct AssetTrajectory {
    int assetId;
    QString assetName;
    QVector<TrajectoryPoint> points;
    float colorHue;
};

class TrajectoryModel : public QAbstractListModel {
    Q_OBJECT
    
public:
    enum Roles {
        AssetIdRole = Qt::UserRole + 1,
        AssetNameRole,
        PointsRole,
        ColorHueRole
    };
    
    explicit TrajectoryModel(QObject* parent = nullptr);
    
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;
    
    void updateTrajectory(int assetId, const QString& name, const QVector<TrajectoryPoint>& points);
    void removeTrajectory(int assetId);
    void clear();
    void updateAlpha(double currentTime);
    
signals:
    void trajectoryUpdated(int assetId);
    
private:
    QVector<AssetTrajectory> trajectories;
    QMap<int, int> idToIndex;
};