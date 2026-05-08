#include "trajectory_query.h"

namespace spatial_query {

TrajectoryQuery& TrajectoryQuery::instance() {
    static TrajectoryQuery query;
    return query;
}

void TrajectoryQuery::addPoint(const TrajectoryPoint& point) {
    trajectories_[point.asset_id].push_back(point);
}

TrajectoryQueryResult TrajectoryQuery::queryAssetTrajectory(uint64_t asset_id, uint64_t start_time, uint64_t end_time) {
    TrajectoryQueryResult result;
    result.asset_id = asset_id;
    
    auto it = trajectories_.find(asset_id);
    if (it == trajectories_.end()) {
        return result;
    }
    
    for (const auto& point : it->second) {
        if (point.timestamp >= start_time && point.timestamp <= end_time) {
            result.points.push_back(point);
        }
    }
    
    return result;
}

TrajectoryQueryResult TrajectoryQuery::queryAssetRecent(uint64_t asset_id, uint64_t duration_ms) {
    uint64_t end_time = 0;
    uint64_t start_time = end_time - duration_ms;
    return queryAssetTrajectory(asset_id, start_time, end_time);
}

std::vector<TrajectoryPoint> TrajectoryQuery::queryZoneTrajectory(uint64_t zone_id, uint64_t start_time, uint64_t end_time) {
    std::vector<TrajectoryPoint> result;
    
    for (const auto& pair : trajectories_) {
        for (const auto& point : pair.second) {
            if (point.timestamp >= start_time && point.timestamp <= end_time) {
                result.push_back(point);
            }
        }
    }
    
    return result;
}

size_t TrajectoryQuery::getPointCount(uint64_t asset_id) const {
    auto it = trajectories_.find(asset_id);
    return (it != trajectories_.end()) ? it->second.size() : 0;
}

void TrajectoryQuery::clearAssetHistory(uint64_t asset_id) {
    trajectories_.erase(asset_id);
}

void TrajectoryQuery::clearAllHistory() {
    trajectories_.clear();
}

} // namespace spatial_query