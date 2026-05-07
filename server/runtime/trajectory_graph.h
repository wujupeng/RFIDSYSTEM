#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>
#include <mutex>

struct TrajectoryNode {
    int32_t zone_x;
    int32_t zone_y;
    uint64_t visit_count;
    float avg_dwell_time_ms;
    uint64_t last_visit_timestamp;
};

struct TrajectoryEdge {
    int32_t from_zone_x;
    int32_t from_zone_y;
    int32_t to_zone_x;
    int32_t to_zone_y;
    uint64_t transition_count;
    float transition_probability;
    float avg_transition_time_ms;
};

struct TrajectoryPrediction {
    int32_t current_zone_x;
    int32_t current_zone_y;
    int32_t predicted_zone_x;
    int32_t predicted_zone_y;
    float confidence;
    uint64_t prediction_horizon_ms;
};

class TrajectoryGraph {
public:
    TrajectoryGraph();
    ~TrajectoryGraph();
    
    void initialize(int gridSize, float cellSize);
    void shutdown();
    
    void updateAssetPosition(uint64_t assetId, float x, float y, uint64_t timestamp);
    void finalizeFrame(uint64_t timestamp);
    
    TrajectoryPrediction predictNextZone(uint64_t assetId, uint64_t currentTime) const;
    std::vector<TrajectoryPrediction> predictTopKZones(uint64_t assetId, uint64_t currentTime, int k) const;
    
    const std::vector<TrajectoryEdge>& getEdges() const;
    float getZoneTransitionProbability(int32_t fromX, int32_t fromY, int32_t toX, int32_t toY) const;
    
private:
    int32_t worldToZone(float worldCoord) const;
    
    void updateEdge(int32_t fromX, int32_t fromY, int32_t toX, int32_t toY, uint64_t timestamp);
    void computeTransitionProbabilities();
    
    int grid_size_;
    float cell_size_;
    
    std::unordered_map<uint64_t, std::pair<int32_t, int32_t>> last_positions_;
    std::unordered_map<uint64_t, uint64_t> last_timestamps_;
    
    std::vector<TrajectoryNode> nodes_;
    std::vector<TrajectoryEdge> edges_;
    
    mutable std::mutex mutex_;
    
    static constexpr uint64_t MIN_TRANSITION_INTERVAL_MS = 1000;
};