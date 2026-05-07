#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <map>

// Runtime ABI - 全系统唯一时空数据格式
// 后面不要乱改

struct AssetPosition {
    int32_t asset_id;
    std::string asset_name;
    std::string tag_id;
    double x;
    double y;
    double vx;
    double vy;
    double risk_score;
    int32_t state;
    float size;
    
    // Bandit info
    std::map<std::string, double> action_scores;
    std::string top_action;
    double top_score;
    
    // Prediction
    double predicted_x;
    double predicted_y;
    double prediction_confidence;
};

struct HeatCell {
    int32_t x;
    int32_t y;
    float value;
};

struct CongestionZone {
    int32_t id;
    std::string name;
    float x;
    float y;
    float radius;
    float density;
    int32_t asset_count;
    std::string level; // "NORMAL", "HIGH_DENSITY", "CONGESTED"
};

struct DecisionPoint {
    int32_t asset_id;
    std::string action;
    double confidence;
    double expected_reward;
    std::string reason;
};

struct PathPrediction {
    int32_t asset_id;
    double x;
    double y;
    double confidence;
    int64_t timestamp;
};

// AI Decision Overlay - POD structure for GPU upload
struct DecisionOverlay {
    uint64_t asset_id;
    
    float x;
    float y;
    
    uint32_t action;       // 0=NO_ACTION, 1=INSPECT, 2=ALERT, 3=REALLOCATE
    float confidence;      // 0~1
    
    float missing_risk;
    float abnormal_risk;
    float inactivity_risk;
    
    char reason[128];
    
    uint64_t timestamp;
};

struct SpatialFrame {
    int64_t timestamp;
    uint64_t frame_id;
    
    std::vector<AssetPosition> assets;
    std::vector<HeatCell> heatmap;
    std::vector<CongestionZone> congestion;
    std::vector<DecisionPoint> decisions;
    std::vector<PathPrediction> predictions;
    std::vector<DecisionOverlay> overlays;
    
    // Frame metadata
    double processing_time_ms;
    bool is_key_frame;
    uint64_t decision_hash;
    
    // Frame budget tracking
    double collect_ms;
    double heatmap_ms;
    double congestion_ms;
    double decision_ms;
    double overlay_ms;
    double upload_ms;
    double render_ms;
    
    void clear() {
        timestamp = 0;
        frame_id = 0;
        assets.clear();
        heatmap.clear();
        congestion.clear();
        decisions.clear();
        predictions.clear();
        overlays.clear();
        processing_time_ms = 0.0;
        is_key_frame = false;
        decision_hash = 0;
        
        collect_ms = 0.0;
        heatmap_ms = 0.0;
        congestion_ms = 0.0;
        decision_ms = 0.0;
        overlay_ms = 0.0;
        upload_ms = 0.0;
        render_ms = 0.0;
    }
    
    size_t totalSize() const {
        return assets.size() + heatmap.size() + congestion.size() + decisions.size() + predictions.size() + overlays.size();
    }
    
    // Deterministic hash for verification
    uint64_t hash() const;
};