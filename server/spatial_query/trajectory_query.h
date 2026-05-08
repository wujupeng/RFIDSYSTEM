#pragma once

#include <cstdint>
#include <vector>
#include <map>

namespace spatial_query {

struct TrajectoryPoint {
    uint64_t asset_id;
    float x;
    float y;
    float z;
    uint64_t timestamp;
    float confidence;
};

struct TrajectoryQueryResult {
    uint64_t asset_id;
    std::string epc;
    std::vector<TrajectoryPoint> points;
};

class TrajectoryQuery {
public:
    static TrajectoryQuery& instance();
    
    void addPoint(const TrajectoryPoint& point);
    
    TrajectoryQueryResult queryAssetTrajectory(uint64_t asset_id, uint64_t start_time, uint64_t end_time);
    
    TrajectoryQueryResult queryAssetRecent(uint64_t asset_id, uint64_t duration_ms);
    
    std::vector<TrajectoryPoint> queryZoneTrajectory(uint64_t zone_id, uint64_t start_time, uint64_t end_time);
    
    size_t getPointCount(uint64_t asset_id) const;
    
    void clearAssetHistory(uint64_t asset_id);
    
    void clearAllHistory();
    
private:
    TrajectoryQuery() = default;
    
    std::map<uint64_t, std::vector<TrajectoryPoint>> trajectories_;
};

} // namespace spatial_query