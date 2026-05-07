#include "trajectory_model.h"

TrajectoryModel::TrajectoryModel(QObject* parent) 
    : QAbstractListModel(parent) {
}

int TrajectoryModel::rowCount(const QModelIndex& parent) const {
    Q_UNUSED(parent);
    return trajectories.size();
}

QVariant TrajectoryModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() >= trajectories.size()) {
        return QVariant();
    }
    
    const AssetTrajectory& traj = trajectories[index.row()];
    
    switch (role) {
        case AssetIdRole:
            return traj.assetId;
        case AssetNameRole:
            return traj.assetName;
        case ColorHueRole:
            return traj.colorHue;
        default:
            return QVariant();
    }
}

QHash<int, QByteArray> TrajectoryModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[AssetIdRole] = "assetId";
    roles[AssetNameRole] = "assetName";
    roles[PointsRole] = "points";
    roles[ColorHueRole] = "colorHue";
    return roles;
}

void TrajectoryModel::updateTrajectory(int assetId, const QString& name, const QVector<TrajectoryPoint>& points) {
    if (idToIndex.contains(assetId)) {
        int index = idToIndex[assetId];
        trajectories[index].points = points;
        trajectories[index].assetName = name;
        emit dataChanged(createIndex(index, 0), createIndex(index, 0));
        emit trajectoryUpdated(assetId);
    } else {
        beginInsertRows(QModelIndex(), trajectories.size(), trajectories.size());
        AssetTrajectory traj;
        traj.assetId = assetId;
        traj.assetName = name;
        traj.points = points;
        traj.colorHue = static_cast<float>(assetId % 360) / 360.0f;
        idToIndex[assetId] = trajectories.size();
        trajectories.append(traj);
        endInsertRows();
        emit trajectoryUpdated(assetId);
    }
}

void TrajectoryModel::removeTrajectory(int assetId) {
    if (!idToIndex.contains(assetId)) return;
    
    int index = idToIndex[assetId];
    beginRemoveRows(QModelIndex(), index, index);
    trajectories.remove(index);
    idToIndex.remove(assetId);
    
    for (auto it = idToIndex.begin(); it != idToIndex.end(); ++it) {
        if (it.value() > index) {
            it.value()--;
        }
    }
    
    endRemoveRows();
}

void TrajectoryModel::clear() {
    beginResetModel();
    trajectories.clear();
    idToIndex.clear();
    endResetModel();
}

void TrajectoryModel::updateAlpha(double currentTime) {
    for (auto& traj : trajectories) {
        for (auto& point : traj.points) {
            double delta = currentTime - point.timestamp;
            point.alpha = static_cast<float>(exp(-delta * 0.5));
        }
    }
}