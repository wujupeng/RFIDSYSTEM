#pragma once

#include "ai_analysis_context.h"
#include <vector>
#include <mutex>

class RiskField {
public:
    RiskField();
    ~RiskField();
    
    void initialize(int gridSize, float cellSize);
    void shutdown();
    
    void updateFromAssets(const std::vector<std::pair<uint64_t, float>>& assetRisks,
                         const std::vector<std::pair<float, float>>& assetPositions);
    
    void computePropagation();
    
    float getCellRisk(int32_t x, int32_t y) const;
    float getCellPropagationRisk(int32_t x, int32_t y) const;
    
    const std::vector<RiskCell>& getRiskMap() const;
    
    float getGlobalRiskIndex() const;
    float getPropagationIntensity() const;
    
private:
    int32_t worldToCell(float worldCoord) const;
    float computeDistance(int32_t x1, int32_t y1, int32_t x2, int32_t y2) const;
    
    int grid_size_;
    float cell_size_;
    std::vector<RiskCell> risk_map_;
    
    float global_risk_index_;
    float propagation_intensity_;
    
    mutable std::mutex mutex_;
    
    static constexpr float DECAY_FACTOR = 0.3f;
    static constexpr float PROPAGATION_RADIUS = 3.0f;
};