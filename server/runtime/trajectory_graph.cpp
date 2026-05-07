#include "trajectory_graph.h"
#include <algorithm>
#include <cmath>

TrajectoryGraph::TrajectoryGraph()
    : grid_size_(64), cell_size_(10.0f) {
}

TrajectoryGraph::~TrajectoryGraph() {
    shutdown();
}

void TrajectoryGraph::initialize(int gridSize, float cellSize) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    grid_size_ = gridSize;
    cell_size_ = cellSize;
    
    nodes_.clear();
    nodes_.resize(grid_size_ * grid_size_);
    
    for (int i = 0; i < grid_size_; ++i) {
        for (int j = 0; j < grid_size_; ++j) {
            TrajectoryNode& node = nodes_[i * grid_size_ + j];
            node.zone_x = i;
            node.zone_y = j;
            node.visit_count = 0;
            node.avg_dwell_time_ms = 0.0f;
            node.last_visit_timestamp = 0;
        }
    }
    
    edges_.clear();
}

void TrajectoryGraph::shutdown() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    nodes_.clear();
    edges_.clear();
    last_positions_.clear();
    last_timestamps_.clear();
}

void TrajectoryGraph::updateAssetPosition(uint64_t assetId, float x, float y, uint64_t timestamp) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    int32_t zoneX = worldToZone(x);
    int32_t zoneY = worldToZone(y);
    
    auto it = last_positions_.find(assetId);
    if (it != last_positions_.end()) {
        int32_t lastX = it->second.first;
        int32_t lastY = it->second.second;
        
        if (lastX != zoneX || lastY != zoneY) {
            updateEdge(lastX, lastY, zoneX, zoneY, timestamp);
        }
    }
    
    last_positions_[assetId] = {zoneX, zoneY};
    last_timestamps_[assetId] = timestamp;
    
    size_t nodeIdx = static_cast<size_t>(zoneX) * grid_size_ + zoneY;
    if (nodeIdx < nodes_.size()) {
        TrajectoryNode& node = nodes_[nodeIdx];
        
        uint64_t now = timestamp;
        if (node.last_visit_timestamp > 0) {
            uint64_t dwell = now - node.last_visit_timestamp;
            node.avg_dwell_time_ms = (node.avg_dwell_time_ms * (node.visit_count - 1) + dwell) / node.visit_count;
        }
        
        node.visit_count++;
        node.last_visit_timestamp = now;
    }
}

void TrajectoryGraph::finalizeFrame(uint64_t timestamp) {
    computeTransitionProbabilities();
}

TrajectoryPrediction TrajectoryGraph::predictNextZone(uint64_t assetId, uint64_t currentTime) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    TrajectoryPrediction result;
    result.confidence = 0.0f;
    result.prediction_horizon_ms = 3000;
    
    auto posIt = last_positions_.find(assetId);
    if (posIt == last_positions_.end()) {
        return result;
    }
    
    result.current_zone_x = posIt->second.first;
    result.current_zone_y = posIt->second.second;
    
    float maxProb = 0.0f;
    int32_t bestX = result.current_zone_x;
    int32_t bestY = result.current_zone_y;
    
    for (const auto& edge : edges_) {
        if (edge.from_zone_x == result.current_zone_x && 
            edge.from_zone_y == result.current_zone_y) {
            if (edge.transition_probability > maxProb) {
                maxProb = edge.transition_probability;
                bestX = edge.to_zone_x;
                bestY = edge.to_zone_y;
            }
        }
    }
    
    result.predicted_zone_x = bestX;
    result.predicted_zone_y = bestY;
    result.confidence = maxProb;
    
    return result;
}

std::vector<TrajectoryPrediction> TrajectoryGraph::predictTopKZones(uint64_t assetId, uint64_t currentTime, int k) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::vector<TrajectoryPrediction> predictions;
    
    auto posIt = last_positions_.find(assetId);
    if (posIt == last_positions_.end()) {
        return predictions;
    }
    
    int32_t currentX = posIt->second.first;
    int32_t currentY = posIt->second.second;
    
    std::vector<std::pair<TrajectoryEdge, float>> candidates;
    
    for (const auto& edge : edges_) {
        if (edge.from_zone_x == currentX && edge.from_zone_y == currentY) {
            candidates.push_back({edge, edge.transition_probability});
        }
    }
    
    std::sort(candidates.begin(), candidates.end(),
             [](const auto& a, const auto& b) { return a.second > b.second; });
    
    size_t count = std::min(static_cast<size_t>(k), candidates.size());
    for (size_t i = 0; i < count; ++i) {
        TrajectoryPrediction pred;
        pred.current_zone_x = currentX;
        pred.current_zone_y = currentY;
        pred.predicted_zone_x = candidates[i].first.to_zone_x;
        pred.predicted_zone_y = candidates[i].first.to_zone_y;
        pred.confidence = candidates[i].second;
        pred.prediction_horizon_ms = 3000;
        
        predictions.push_back(pred);
    }
    
    return predictions;
}

const std::vector<TrajectoryEdge>& TrajectoryGraph::getEdges() const {
    return edges_;
}

float TrajectoryGraph::getZoneTransitionProbability(int32_t fromX, int32_t fromY, int32_t toX, int32_t toY) const {
    for (const auto& edge : edges_) {
        if (edge.from_zone_x == fromX && edge.from_zone_y == fromY &&
            edge.to_zone_x == toX && edge.to_zone_y == toY) {
            return edge.transition_probability;
        }
    }
    return 0.0f;
}

int32_t TrajectoryGraph::worldToZone(float worldCoord) const {
    int32_t zone = static_cast<int32_t>(worldCoord / cell_size_);
    return std::max(0, std::min(grid_size_ - 1, zone));
}

void TrajectoryGraph::updateEdge(int32_t fromX, int32_t fromY, int32_t toX, int32_t toY, uint64_t timestamp) {
    for (auto& edge : edges_) {
        if (edge.from_zone_x == fromX && edge.from_zone_y == fromY &&
            edge.to_zone_x == toX && edge.to_zone_y == toY) {
            edge.transition_count++;
            return;
        }
    }
    
    TrajectoryEdge newEdge;
    newEdge.from_zone_x = fromX;
    newEdge.from_zone_y = fromY;
    newEdge.to_zone_x = toX;
    newEdge.to_zone_y = toY;
    newEdge.transition_count = 1;
    newEdge.transition_probability = 0.0f;
    newEdge.avg_transition_time_ms = 0.0f;
    
    edges_.push_back(newEdge);
}

void TrajectoryGraph::computeTransitionProbabilities() {
    std::unordered_map<int64_t, uint64_t> fromCounts;
    
    for (const auto& edge : edges_) {
        int64_t key = static_cast<int64_t>(edge.from_zone_x) * 1000 + edge.from_zone_y;
        fromCounts[key] += edge.transition_count;
    }
    
    for (auto& edge : edges_) {
        int64_t key = static_cast<int64_t>(edge.from_zone_x) * 1000 + edge.from_zone_y;
        auto it = fromCounts.find(key);
        if (it != fromCounts.end() && it->second > 0) {
            edge.transition_probability = static_cast<float>(edge.transition_count) / it->second;
        }
    }
}