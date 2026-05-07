#include "heatmap_model.h"

HeatmapModel::HeatmapModel(QObject* parent) 
    : QAbstractListModel(parent), maxValue(1.0f) {
}

int HeatmapModel::rowCount(const QModelIndex& parent) const {
    Q_UNUSED(parent);
    return heatCells.size();
}

QVariant HeatmapModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() >= heatCells.size()) {
        return QVariant();
    }
    
    const HeatCell& cell = heatCells[index.row()];
    
    switch (role) {
        case XRole:
            return cell.x;
        case YRole:
            return cell.y;
        case ValueRole:
            return cell.value;
        case NormalizedRole:
            return cell.normalizedValue;
        default:
            return QVariant();
    }
}

QHash<int, QByteArray> HeatmapModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[XRole] = "x";
    roles[YRole] = "y";
    roles[ValueRole] = "value";
    roles[NormalizedRole] = "normalizedValue";
    return roles;
}

void HeatmapModel::updateHeatmap(const QVector<HeatCell>& cells) {
    beginResetModel();
    heatCells = cells;
    
    maxValue = 1.0f;
    for (const HeatCell& cell : heatCells) {
        if (cell.value > maxValue) {
            maxValue = cell.value;
        }
    }
    
    for (HeatCell& cell : heatCells) {
        cell.normalizedValue = cell.value / maxValue;
    }
    
    endResetModel();
    emit heatmapUpdated();
}

void HeatmapModel::clear() {
    beginResetModel();
    heatCells.clear();
    maxValue = 1.0f;
    endResetModel();
    emit heatmapUpdated();
}

int HeatmapModel::congestionCount() const {
    return congestionZones.size();
}

const CongestionZone& HeatmapModel::congestionAt(int index) const {
    return congestionZones[index];
}

void HeatmapModel::updateCongestion(const QVector<CongestionZone>& zones) {
    congestionZones = zones;
    
    for (CongestionZone& zone : congestionZones) {
        zone.status = getStatusFromScore(zone.score);
    }
    
    emit congestionUpdated();
}

void HeatmapModel::clearCongestion() {
    congestionZones.clear();
    emit congestionUpdated();
}

QString HeatmapModel::getStatusFromScore(float score) const {
    if (score < 0.3) {
        return "normal";
    } else if (score < 0.6) {
        return "high";
    } else if (score < 0.9) {
        return "congested";
    } else {
        return "severe";
    }
}