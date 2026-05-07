#pragma once

#include <cstdint>
#include <vector>
#include <map>
#include <chrono>

struct AIFrameContext {
    float global_risk_index;
    float system_congestion;
    float anomaly_score;
    float prediction_confidence;
    uint32_t high_risk_asset_count;
    uint32_t total_asset_count;
    uint64_t timestamp;
};

struct RiskCell {
    int32_t x;
    int32_t y;
    float risk_value;
    float propagation_risk;
    uint32_t source_asset_count;
};

struct ZonePrediction {
    int32_t zone_id;
    int32_t predicted_x;
    int32_t predicted_y;
    float confidence;
    float risk_level;
    uint64_t prediction_horizon_ms;
};

struct AssetInfluence {
    uint64_t asset_id;
    float influence_radius;
    float risk_contribution;
    std::vector<uint64_t> affected_zone_ids;
};

class AIAnalysisPass {
public:
    AIAnalysisPass();
    ~AIAnalysisPass();
    
    void initialize();
    void shutdown();
    
    void analyze(const std::vector<RiskCell>& riskMap,
                 const std::vector<ZonePrediction>& predictions);
    
    const AIFrameContext& getContext() const;
    const std::vector<RiskCell>& getRiskPropagationMap() const;
    const std::vector<AssetInfluence>& getAssetInfluences() const;
    
private:
    void computeGlobalRiskIndex();
    void computeSystemCongestion();
    void computeAnomalyScore();
    void propagateRisk();
    void computeAssetInfluences();
    
    AIFrameContext context_;
    std::vector<RiskCell> risk_propagation_map_;
    std::vector<AssetInfluence> asset_influences_;
    
    static constexpr float RISK_DECAY_FACTOR = 0.85f;
    static constexpr float ANOMALY_THRESHOLD = 0.7f;
};