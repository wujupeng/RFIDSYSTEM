#pragma once

#include <cstdint>
#include <vector>
#include <map>
#include "spatial_index_v2.h"
#include "geofence_query.h"
#include "trajectory_query.h"

namespace spatial_query {

struct HeatmapCell {
    float x;
    float y;
    float density;
    uint64_t count;
};

struct QueryStatistics {
    uint64_t query_count;
    uint64_t result_count;
    double avg_latency_ms;
};

class SpatialQueryEngine {
public:
    static SpatialQueryEngine& instance();
    
    void initialize();
    
    void shutdown();
    
    void tick();
    
    std::vector<AssetInZone> queryGeofence(uint64_t zone_id);
    
    TrajectoryQueryResult queryTrajectory(uint64_t asset_id, uint64_t start_time, uint64_t end_time);
    
    std::vector<SpatialPoint> queryNearby(float x, float y, float radius);
    
    std::vector<HeatmapCell> queryHeatmap(const SpatialRect& area, float cell_size);
    
    void updateAssetPosition(uint64_t asset_id, float x, float y, float z, uint64_t timestamp);
    
    void updateAssetTrajectory(uint64_t asset_id, float x, float y, float z, uint64_t timestamp);
    
    QueryStatistics getStatistics() const;
    
    void clearCache();
    
private:
    SpatialQueryEngine() = default;
    
    void updateStatistics(uint64_t result_count, double latency_ms);
    
    QueryStatistics stats_;
};

} // namespace spatial_query