#include "spatial_frame.h"
#include <cstring>
#include <functional>

namespace {

// Simple but deterministic hash combining method
// Based on Boost's hash_combine
template <typename T>
void hashCombine(uint64_t& seed, const T& val) {
    std::hash<T> hasher;
    seed ^= hasher(val) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

// Hash for doubles - handles edge cases
uint64_t hashDouble(double d) {
    uint64_t result;
    std::memcpy(&result, &d, sizeof(d));
    return result;
}

// Hash for floats
uint64_t hashFloat(float f) {
    uint32_t result;
    std::memcpy(&result, &f, sizeof(f));
    return static_cast<uint64_t>(result);
}

// Hash for strings
uint64_t hashString(const std::string& s) {
    uint64_t hash = 0xcbf29ce484222325ULL;
    for (char c : s) {
        hash ^= static_cast<uint64_t>(c);
        hash *= 0x100000001b3ULL;
    }
    return hash;
}

} // namespace

uint64_t SpatialFrame::hash() const {
    uint64_t result = 0;
    
    // Hash frame metadata
    hashCombine(result, timestamp);
    hashCombine(result, frame_id);
    
    // Hash assets (sorted by asset_id for determinism)
    auto sortedAssets = assets;
    std::sort(sortedAssets.begin(), sortedAssets.end(), 
              [](const AssetPosition& a, const AssetPosition& b) {
                  return a.asset_id < b.asset_id;
              });
    
    for (const auto& asset : sortedAssets) {
        hashCombine(result, asset.asset_id);
        hashCombine(result, hashString(asset.asset_name));
        hashCombine(result, hashString(asset.tag_id));
        hashCombine(result, hashDouble(asset.x));
        hashCombine(result, hashDouble(asset.y));
        hashCombine(result, hashDouble(asset.vx));
        hashCombine(result, hashDouble(asset.vy));
        hashCombine(result, hashDouble(asset.risk_score));
        hashCombine(result, asset.state);
        hashCombine(result, hashFloat(asset.size));
        hashCombine(result, hashString(asset.top_action));
        hashCombine(result, hashDouble(asset.top_score));
        hashCombine(result, hashDouble(asset.predicted_x));
        hashCombine(result, hashDouble(asset.predicted_y));
        hashCombine(result, hashDouble(asset.prediction_confidence));
        
        // Hash action_scores (sorted for determinism)
        std::vector<std::pair<std::string, double>> sortedActions(
            asset.action_scores.begin(), asset.action_scores.end());
        std::sort(sortedActions.begin(), sortedActions.end());
        
        for (const auto& pair : sortedActions) {
            hashCombine(result, hashString(pair.first));
            hashCombine(result, hashDouble(pair.second));
        }
    }
    
    // Hash heatmap (sorted by x,y for determinism)
    auto sortedHeatmap = heatmap;
    std::sort(sortedHeatmap.begin(), sortedHeatmap.end(),
              [](const HeatCell& a, const HeatCell& b) {
                  if (a.x != b.x) return a.x < b.x;
                  return a.y < b.y;
              });
    
    for (const auto& cell : sortedHeatmap) {
        hashCombine(result, cell.x);
        hashCombine(result, cell.y);
        hashCombine(result, hashFloat(cell.value));
    }
    
    // Hash congestion zones (sorted by id for determinism)
    auto sortedCongestion = congestion;
    std::sort(sortedCongestion.begin(), sortedCongestion.end(),
              [](const CongestionZone& a, const CongestionZone& b) {
                  return a.id < b.id;
              });
    
    for (const auto& zone : sortedCongestion) {
        hashCombine(result, zone.id);
        hashCombine(result, hashString(zone.name));
        hashCombine(result, hashFloat(zone.x));
        hashCombine(result, hashFloat(zone.y));
        hashCombine(result, hashFloat(zone.radius));
        hashCombine(result, hashFloat(zone.density));
        hashCombine(result, zone.asset_count);
        hashCombine(result, hashString(zone.level));
    }
    
    // Hash decisions (sorted by asset_id for determinism)
    auto sortedDecisions = decisions;
    std::sort(sortedDecisions.begin(), sortedDecisions.end(),
              [](const DecisionPoint& a, const DecisionPoint& b) {
                  return a.asset_id < b.asset_id;
              });
    
    for (const auto& decision : sortedDecisions) {
        hashCombine(result, decision.asset_id);
        hashCombine(result, hashString(decision.action));
        hashCombine(result, hashDouble(decision.confidence));
        hashCombine(result, hashDouble(decision.expected_reward));
        hashCombine(result, hashString(decision.reason));
    }
    
    // Hash predictions (sorted by asset_id for determinism)
    auto sortedPredictions = predictions;
    std::sort(sortedPredictions.begin(), sortedPredictions.end(),
              [](const PathPrediction& a, const PathPrediction& b) {
                  return a.asset_id < b.asset_id;
              });
    
    for (const auto& prediction : sortedPredictions) {
        hashCombine(result, prediction.asset_id);
        hashCombine(result, hashDouble(prediction.x));
        hashCombine(result, hashDouble(prediction.y));
        hashCombine(result, hashDouble(prediction.confidence));
        hashCombine(result, prediction.timestamp);
    }
    
    // Hash decision overlays (sorted by asset_id for determinism)
    auto sortedOverlays = overlays;
    std::sort(sortedOverlays.begin(), sortedOverlays.end(),
              [](const DecisionOverlay& a, const DecisionOverlay& b) {
                  return a.asset_id < b.asset_id;
              });
    
    for (const auto& overlay : sortedOverlays) {
        hashCombine(result, overlay.asset_id);
        hashCombine(result, hashFloat(overlay.x));
        hashCombine(result, hashFloat(overlay.y));
        hashCombine(result, overlay.action);
        hashCombine(result, hashFloat(overlay.confidence));
        hashCombine(result, hashFloat(overlay.missing_risk));
        hashCombine(result, hashFloat(overlay.abnormal_risk));
        hashCombine(result, hashFloat(overlay.inactivity_risk));
        hashCombine(result, overlay.timestamp);
        
        // Hash reason string
        for (int i = 0; i < 128 && overlay.reason[i] != '\0'; i++) {
            hashCombine(result, static_cast<uint64_t>(overlay.reason[i]));
        }
    }
    
    // Hash decision hash
    hashCombine(result, decision_hash);
    
    return result;
}