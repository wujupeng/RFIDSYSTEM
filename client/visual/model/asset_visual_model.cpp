#include "asset_visual_model.h"

AssetVisualModel::AssetVisualModel(QObject* parent) 
    : QAbstractListModel(parent) {
}

int AssetVisualModel::rowCount(const QModelIndex& parent) const {
    Q_UNUSED(parent);
    return assets.size();
}

QVariant AssetVisualModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() >= assets.size()) {
        return QVariant();
    }
    
    const AssetVisual& asset = assets[index.row()];
    
    switch (role) {
        case IdRole:
            return asset.id;
        case NameRole:
            return asset.name;
        case TagIdRole:
            return asset.tagId;
        case XRole:
            return asset.x;
        case YRole:
            return asset.y;
        case VXRole:
            return asset.vx;
        case VYRole:
            return asset.vy;
        case RiskRole:
            return asset.risk;
        case StateRole:
            return static_cast<int>(asset.state);
        case SizeRole:
            return asset.size;
        case HeatRole:
            return asset.heat;
        case TopActionRole:
            return asset.topAction;
        case TopScoreRole:
            return asset.topScore;
        default:
            return QVariant();
    }
}

QHash<int, QByteArray> AssetVisualModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[IdRole] = "id";
    roles[NameRole] = "name";
    roles[TagIdRole] = "tagId";
    roles[XRole] = "x";
    roles[YRole] = "y";
    roles[VXRole] = "vx";
    roles[VYRole] = "vy";
    roles[RiskRole] = "risk";
    roles[StateRole] = "state";
    roles[SizeRole] = "size";
    roles[HeatRole] = "heat";
    roles[TopActionRole] = "topAction";
    roles[TopScoreRole] = "topScore";
    return roles;
}

void AssetVisualModel::updateAsset(const AssetVisual& asset) {
    if (idToIndex.contains(asset.id)) {
        int index = idToIndex[asset.id];
        assets[index] = asset;
        emit dataChanged(createIndex(index, 0), createIndex(index, 0));
        emit assetUpdated(asset.id);
    } else {
        beginInsertRows(QModelIndex(), assets.size(), assets.size());
        idToIndex[asset.id] = assets.size();
        assets.append(asset);
        endInsertRows();
        emit assetUpdated(asset.id);
    }
}

void AssetVisualModel::removeAsset(int id) {
    if (!idToIndex.contains(id)) return;
    
    int index = idToIndex[id];
    beginRemoveRows(QModelIndex(), index, index);
    assets.remove(index);
    idToIndex.remove(id);
    
    for (auto it = idToIndex.begin(); it != idToIndex.end(); ++it) {
        if (it.value() > index) {
            it.value()--;
        }
    }
    
    endRemoveRows();
    emit assetRemoved(id);
}

void AssetVisualModel::clear() {
    beginResetModel();
    assets.clear();
    idToIndex.clear();
    endResetModel();
}

const AssetVisual* AssetVisualModel::getAsset(int id) const {
    if (!idToIndex.contains(id)) return nullptr;
    return &assets[idToIndex[id]];
}