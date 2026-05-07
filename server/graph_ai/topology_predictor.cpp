#include "topology_predictor.h"
#include <cmath>
#include <algorithm>

TopologySnapshot TopologyPredictor::predict(const std::vector<TopologySnapshot>& history,
                                            int future_seconds) {
    if (history.empty()) {
        return TopologySnapshot();
    }
    
    TopologySnapshot prediction = history.back();
    prediction.timestamp = history.back().timestamp + future_seconds * 1000;
    
    for (auto& node : prediction.nodes) {
        node.load = predictNodeLoad(node.reader_id, history);
    }
    
    for (auto& edge : prediction.edges) {
        edge.transition_prob = predictEdgeTraffic(edge.from, edge.to, history);
        edge.strength = 0.4f * edge.overlap + 0.4f * edge.transition_prob + 0.2f * edge.correlation;
    }
    
    return prediction;
}

float TopologyPredictor::predictNodeLoad(uint64_t node_id,
                                         const std::vector<TopologySnapshot>& history) {
    if (history.size() < 2) {
        return history.empty() ? 0.0f : history.back().nodes[0].load;
    }
    
    std::vector<float> loads;
    for (const auto& snap : history) {
        auto it = std::find_if(snap.nodes.begin(), snap.nodes.end(),
            [node_id](const TopologyNode& n) { return n.reader_id == node_id; });
        if (it != snap.nodes.end()) {
            loads.push_back(it->load);
        }
    }
    
    if (loads.size() < 2) return loads.empty() ? 0.0f : loads[0];
    
    float velocity = calculateVelocity(loads);
    float ema = calculateEMA(loads);
    
    return std::max(0.0f, std::min(1.0f, ema + velocity));
}

float TopologyPredictor::predictEdgeTraffic(uint64_t from_id, uint64_t to_id,
                                           const std::vector<TopologySnapshot>& history) {
    if (history.size() < 2) return 0.0f;
    
    std::vector<float> probs;
    for (const auto& snap : history) {
        auto it = std::find_if(snap.edges.begin(), snap.edges.end(),
            [from_id, to_id](const TopologyEdge& e) { 
                return e.from == from_id && e.to == to_id; 
            });
        if (it != snap.edges.end()) {
            probs.push_back(it->transition_prob);
        }
    }
    
    if (probs.size() < 2) return probs.empty() ? 0.0f : probs[0];
    
    float velocity = calculateVelocity(probs);
    float ema = calculateEMA(probs);
    
    return std::max(0.0f, std::min(1.0f, ema + velocity));
}

float TopologyPredictor::predictFailureProbability(uint64_t node_id,
                                                  const std::vector<TopologySnapshot>& history) {
    if (history.size() < 3) return 0.0f;
    
    std::vector<float> loads;
    for (const auto& snap : history) {
        auto it = std::find_if(snap.nodes.begin(), snap.nodes.end(),
            [node_id](const TopologyNode& n) { return n.reader_id == node_id; });
        if (it != snap.nodes.end()) {
            loads.push_back(it->load);
        }
    }
    
    if (loads.size() < 3) return 0.0f;
    
    float velocity = calculateVelocity(loads);
    
    float failure_prob = 0.0f;
    if (loads.back() > 0.8f) failure_prob += 0.3f;
    if (velocity > 0.1f) failure_prob += 0.2f;
    if (loads.size() > 5) {
        float variance = 0.0f;
        float mean = std::accumulate(loads.begin(), loads.end(), 0.0f) / loads.size();
        for (float l : loads) {
            variance += (l - mean) * (l - mean);
        }
        variance /= loads.size();
        if (variance > 0.05f) failure_prob += 0.3f;
    }
    
    return std::min(1.0f, failure_prob);
}

float TopologyPredictor::calculateVelocity(const std::vector<float>& values) {
    if (values.size() < 2) return 0.0f;
    
    float recent_diff = values.back() - values[values.size() - 2];
    float earlier_diff = values.size() > 2 ? values[values.size() - 2] - values[values.size() - 3] : 0.0f;
    
    return (recent_diff + earlier_diff) / 2.0f;
}

float TopologyPredictor::calculateEMA(const std::vector<float>& values) {
    if (values.empty()) return 0.0f;
    
    float alpha = 0.3f;
    float ema = values[0];
    
    for (size_t i = 1; i < values.size(); ++i) {
        ema = alpha * values[i] + (1 - alpha) * ema;
    }
    
    return ema;
}