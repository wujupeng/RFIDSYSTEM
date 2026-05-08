#pragma once

#include <cstdint>
#include <vector>
#include <map>
#include "spatial_index_v2.h"

namespace spatial_query {

enum class GeofenceType {
    RECTANGLE,
    CIRCLE,
    POLYGON
};

struct Geofence {
    uint64_t zone_id;
    std::string name;
    GeofenceType type;
    SpatialRect rect;
    SpatialCircle circle;
    std::vector<std::pair<float, float>> polygon;
};

struct AssetInZone {
    uint64_t asset_id;
    std::string epc;
    float x;
    float y;
    float confidence;
};

class GeofenceQuery {
public:
    static GeofenceQuery& instance();
    
    void addZone(const Geofence& zone);
    
    void removeZone(uint64_t zone_id);
    
    void updateZone(uint64_t zone_id, const Geofence& zone);
    
    std::vector<AssetInZone> queryAssetsInZone(uint64_t zone_id);
    
    std::vector<AssetInZone> queryAssetsInZoneByName(const std::string& name);
    
    std::vector<Geofence> getZones() const;
    
    const Geofence* getZone(uint64_t zone_id) const;
    
    bool isPointInZone(float x, float y, uint64_t zone_id) const;
    
private:
    GeofenceQuery() = default;
    
    bool isPointInPolygon(float x, float y, const std::vector<std::pair<float, float>>& polygon) const;
    
    std::map<uint64_t, Geofence> zones_;
};

} // namespace spatial_query