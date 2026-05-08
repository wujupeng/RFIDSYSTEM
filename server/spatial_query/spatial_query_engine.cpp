#include "spatial_query_engine.h"

namespace spatial_query {

SpatialQueryEngine& SpatialQueryEngine::instance() {
    static SpatialQueryEngine engine;
    return engine;
}

void SpatialQueryEngine::initialize() {
    stats_ = {};
}

void SpatialQueryEngine::shutdown() {
    clearCache();
}

void SpatialQueryEngine::tick() {
}

std::vector<AssetInZone> SpatialQueryEngine::queryGeofence(uint64_t zone_id) {
    return GeofenceQuery::instance().queryAssetsInZone(zone_id);
}

TrajectoryQueryResult SpatialQueryEngine::queryTrajectory(uint64_t asset_id, uint64_t start_time, uint64_t end_time) {
    return TrajectoryQuery::instance().queryAssetTrajectory(asset_id, start_time, end_time);
}

std::vector<SpatialPoint> SpatialQueryEngine::queryNearby(float x, float y, float radius) {
    return SpatialIndexV2::instance().queryNearby(x, y, radius);
}

std::vector<HeatmapCell> SpatialQueryEngine::queryHeatmap(const SpatialRect& area, float cell_size) {
    std::vector<HeatmapCell> result;
    
    int cells_x = static_cast<int>((area.max_x - area.min_x) / cell_size) + 1;
    int cells_y = static_cast<int>((area.max_y - area.min_y) / cell_size) + 1;
    
    std::vector<std::vector<int>> grid(cells_y, std::vector<int>(cells_x, 0));
    
    auto points = SpatialIndexV2::instance().queryRect(area);
    for (const auto& point : points) {
        int ix = static_cast<int>((point.x - area.min_x) / cell_size);
        int iy = static_cast<int>((point.y - area.min_y) / cell_size);
        
        if (ix >= 0 && ix < cells_x && iy >= 0 && iy < cells_y) {
            grid[iy][ix]++;
        }
    }
    
    for (int iy = 0; iy < cells_y; ++iy) {
        for (int ix = 0; ix < cells_x; ++ix) {
            HeatmapCell cell;
            cell.x = area.min_x + ix * cell_size;
            cell.y = area.min_y + iy * cell_size;
            cell.count = grid[iy][ix];
            cell.density = static_cast<float>(grid[iy][ix]) / static_cast<float>(points.size());
            result.push_back(cell);
        }
    }
    
    return result;
}

void SpatialQueryEngine::updateAssetPosition(uint64_t asset_id, float x, float y, float z, uint64_t timestamp) {
    SpatialPoint point;
    point.id = asset_id;
    point.x = x;
    point.y = y;
    point.z = z;
    point.timestamp = timestamp;
    
    SpatialIndexV2::instance().update(asset_id, x, y, z);
}

void SpatialQueryEngine::updateAssetTrajectory(uint64_t asset_id, float x, float y, float z, uint64_t timestamp) {
    TrajectoryPoint point;
    point.asset_id = asset_id;
    point.x = x;
    point.y = y;
    point.z = z;
    point.timestamp = timestamp;
    point.confidence = 1.0f;
    
    TrajectoryQuery::instance().addPoint(point);
}

QueryStatistics SpatialQueryEngine::getStatistics() const {
    return stats_;
}

void SpatialQueryEngine::clearCache() {
    SpatialIndexV2::instance().clear();
}

void SpatialQueryEngine::updateStatistics(uint64_t result_count, double latency_ms) {
    stats_.query_count++;
    stats_.result_count += result_count;
    stats_.avg_latency_ms = (stats_.avg_latency_ms * (stats_.query_count - 1) + latency_ms) / stats_.query_count;
}

} // namespace spatial_query