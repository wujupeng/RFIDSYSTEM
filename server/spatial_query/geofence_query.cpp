#include "geofence_query.h"

namespace spatial_query {

GeofenceQuery& GeofenceQuery::instance() {
    static GeofenceQuery query;
    return query;
}

void GeofenceQuery::addZone(const Geofence& zone) {
    zones_[zone.zone_id] = zone;
}

void GeofenceQuery::removeZone(uint64_t zone_id) {
    zones_.erase(zone_id);
}

void GeofenceQuery::updateZone(uint64_t zone_id, const Geofence& zone) {
    zones_[zone_id] = zone;
}

std::vector<AssetInZone> GeofenceQuery::queryAssetsInZone(uint64_t zone_id) {
    auto it = zones_.find(zone_id);
    if (it == zones_.end()) {
        return {};
    }
    
    const Geofence& zone = it->second;
    std::vector<AssetInZone> result;
    
    switch (zone.type) {
        case GeofenceType::RECTANGLE: {
            auto points = SpatialIndexV2::instance().queryRect(zone.rect);
            for (const auto& point : points) {
                AssetInZone asset;
                asset.asset_id = point.id;
                asset.x = point.x;
                asset.y = point.y;
                asset.confidence = 1.0f;
                result.push_back(asset);
            }
            break;
        }
        case GeofenceType::CIRCLE: {
            auto points = SpatialIndexV2::instance().queryCircle(zone.circle);
            for (const auto& point : points) {
                AssetInZone asset;
                asset.asset_id = point.id;
                asset.x = point.x;
                asset.y = point.y;
                asset.confidence = 1.0f;
                result.push_back(asset);
            }
            break;
        }
        case GeofenceType::POLYGON: {
            auto all_points = SpatialIndexV2::instance().queryNearby(
                zone.polygon[0].first, zone.polygon[0].second, 100.0f);
            for (const auto& point : all_points) {
                if (isPointInPolygon(point.x, point.y, zone.polygon)) {
                    AssetInZone asset;
                    asset.asset_id = point.id;
                    asset.x = point.x;
                    asset.y = point.y;
                    asset.confidence = 1.0f;
                    result.push_back(asset);
                }
            }
            break;
        }
    }
    
    return result;
}

std::vector<AssetInZone> GeofenceQuery::queryAssetsInZoneByName(const std::string& name) {
    for (const auto& pair : zones_) {
        if (pair.second.name == name) {
            return queryAssetsInZone(pair.first);
        }
    }
    return {};
}

std::vector<Geofence> GeofenceQuery::getZones() const {
    std::vector<Geofence> result;
    for (const auto& pair : zones_) {
        result.push_back(pair.second);
    }
    return result;
}

const Geofence* GeofenceQuery::getZone(uint64_t zone_id) const {
    auto it = zones_.find(zone_id);
    return (it != zones_.end()) ? &it->second : nullptr;
}

bool GeofenceQuery::isPointInZone(float x, float y, uint64_t zone_id) const {
    auto it = zones_.find(zone_id);
    if (it == zones_.end()) {
        return false;
    }
    
    const Geofence& zone = it->second;
    
    switch (zone.type) {
        case GeofenceType::RECTANGLE:
            return x >= zone.rect.min_x && x <= zone.rect.max_x &&
                   y >= zone.rect.min_y && y <= zone.rect.max_y;
        case GeofenceType::CIRCLE: {
            float dx = x - zone.circle.center_x;
            float dy = y - zone.circle.center_y;
            return dx * dx + dy * dy <= zone.circle.radius * zone.circle.radius;
        }
        case GeofenceType::POLYGON:
            return isPointInPolygon(x, y, zone.polygon);
    }
    
    return false;
}

bool GeofenceQuery::isPointInPolygon(float x, float y, const std::vector<std::pair<float, float>>& polygon) const {
    bool inside = false;
    size_t n = polygon.size();
    for (size_t i = 0, j = n - 1; i < n; j = i++) {
        float xi = polygon[i].first, yi = polygon[i].second;
        float xj = polygon[j].first, yj = polygon[j].second;
        
        if (((yi > y) != (yj > y)) &&
            (x < (xj - xi) * (y - yi) / (yj - yi) + xi)) {
            inside = !inside;
        }
    }
    return inside;
}

} // namespace spatial_query