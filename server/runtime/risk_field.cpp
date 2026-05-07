#include "risk_field.h"
#include <cmath>
#include <algorithm>

RiskField::RiskField()
    : grid_size_(64), cell_size_(10.0f),
      global_risk_index_(0.0f), propagation_intensity_(0.0f) {
}

RiskField::~RiskField() {
    shutdown();
}

void RiskField::initialize(int gridSize, float cellSize) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    grid_size_ = gridSize;
    cell_size_ = cellSize;
    
    risk_map_.resize(grid_size_ * grid_size_);
    
    for (int i = 0; i < grid_size_; ++i) {
        for (int j = 0; j < grid_size_; ++j) {
            RiskCell& cell = risk_map_[i * grid_size_ + j];
            cell.x = i;
            cell.y = j;
            cell.risk_value = 0.0f;
            cell.propagation_risk = 0.0f;
            cell.source_asset_count = 0;
        }
    }
}

void RiskField::shutdown() {
    std::lock_guard<std::mutex> lock(mutex_);
    risk_map_.clear();
}

void RiskField::updateFromAssets(const std::vector<std::pair<uint64_t, float>>& assetRisks,
                                const std::vector<std::pair<float, float>>& assetPositions) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    for (auto& cell : risk_map_) {
        cell.risk_value = 0.0f;
        cell.source_asset_count = 0;
    }
    
    size_t count = std::min(assetRisks.size(), assetPositions.size());
    
    for (size_t i = 0; i < count; ++i) {
        uint64_t assetId = assetRisks[i].first;
        float risk = assetRisks[i].second;
        float x = assetPositions[i].first;
        float y = assetPositions[i].second;
        
        int32_t cellX = worldToCell(x);
        int32_t cellY = worldToCell(y);
        
        if (cellX >= 0 && cellX < grid_size_ && cellY >= 0 && cellY < grid_size_) {
            RiskCell& cell = risk_map_[cellX * grid_size_ + cellY];
            cell.risk_value = std::max(cell.risk_value, risk);
            cell.source_asset_count++;
        }
    }
}

void RiskField::computePropagation() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    for (auto& cell : risk_map_) {
        cell.propagation_risk = cell.risk_value;
    }
    
    for (size_t i = 0; i < risk_map_.size(); ++i) {
        const RiskCell& source = risk_map_[i];
        if (source.risk_value < 0.01f) continue;
        
        for (size_t j = 0; j < risk_map_.size(); ++j) {
            RiskCell& target = risk_map_[j];
            if (i == j) continue;
            
            float distance = computeDistance(source.x, source.y, target.x, target.y);
            
            if (distance < PROPAGATION_RADIUS) {
                float propagation = source.risk_value * std::exp(-DECAY_FACTOR * distance);
                target.propagation_risk = std::max(target.propagation_risk, propagation);
            }
        }
    }
    
    float sumRisk = 0.0f;
    float sumPropagation = 0.0f;
    float maxRisk = 0.0f;
    float maxPropagation = 0.0f;
    
    for (const auto& cell : risk_map_) {
        sumRisk += cell.risk_value;
        sumPropagation += cell.propagation_risk;
        maxRisk = std::max(maxRisk, cell.risk_value);
        maxPropagation = std::max(maxPropagation, cell.propagation_risk);
    }
    
    if (!risk_map_.empty()) {
        global_risk_index_ = (sumRisk / risk_map_.size() * 0.6f + maxRisk * 0.4f);
        propagation_intensity_ = (sumPropagation / risk_map_.size() * 0.6f + maxPropagation * 0.4f);
    }
}

float RiskField::getCellRisk(int32_t x, int32_t y) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (x < 0 || x >= grid_size_ || y < 0 || y >= grid_size_) {
        return 0.0f;
    }
    
    return risk_map_[x * grid_size_ + y].risk_value;
}

float RiskField::getCellPropagationRisk(int32_t x, int32_t y) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (x < 0 || x >= grid_size_ || y < 0 || y >= grid_size_) {
        return 0.0f;
    }
    
    return risk_map_[x * grid_size_ + y].propagation_risk;
}

const std::vector<RiskCell>& RiskField::getRiskMap() const {
    return risk_map_;
}

float RiskField::getGlobalRiskIndex() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return global_risk_index_;
}

float RiskField::getPropagationIntensity() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return propagation_intensity_;
}

int32_t RiskField::worldToCell(float worldCoord) const {
    int32_t cell = static_cast<int32_t>(worldCoord / cell_size_);
    return std::max(0, std::min(grid_size_ - 1, cell));
}

float RiskField::computeDistance(int32_t x1, int32_t y1, int32_t x2, int32_t y2) const {
    float dx = static_cast<float>(x1 - x2);
    float dy = static_cast<float>(y1 - y2);
    return std::sqrt(dx * dx + dy * dy);
}