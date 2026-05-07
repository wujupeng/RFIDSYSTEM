#include "ai_analysis_context.h"
#include <cmath>
#include <algorithm>

AIAnalysisPass::AIAnalysisPass() {
    context_.global_risk_index = 0.0f;
    context_.system_congestion = 0.0f;
    context_.anomaly_score = 0.0f;
    context_.prediction_confidence = 0.0f;
    context_.high_risk_asset_count = 0;
    context_.total_asset_count = 0;
    context_.timestamp = 0;
}

AIAnalysisPass::~AIAnalysisPass() {
    shutdown();
}

void AIAnalysisPass::initialize() {
    risk_propagation_map_.clear();
    asset_influences_.clear();
}

void AIAnalysisPass::shutdown() {
    risk_propagation_map_.clear();
    asset_influences_.clear();
}

void AIAnalysisPass::analyze(const std::vector<RiskCell>& riskMap,
                            const std::vector<ZonePrediction>& predictions) {
    risk_propagation_map_ = riskMap;
    
    computeGlobalRiskIndex();
    computeSystemCongestion();
    computeAnomalyScore();
    propagateRisk();
    computeAssetInfluences();
    
    context_.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    
    if (!predictions.empty()) {
        float sum_confidence = 0.0f;
        for (const auto& pred : predictions) {
            sum_confidence += pred.confidence;
        }
        context_.prediction_confidence = sum_confidence / predictions.size();
    }
}

const AIFrameContext& AIAnalysisPass::getContext() const {
    return context_;
}

const std::vector<RiskCell>& AIAnalysisPass::getRiskPropagationMap() const {
    return risk_propagation_map_;
}

const std::vector<AssetInfluence>& AIAnalysisPass::getAssetInfluences() const {
    return asset_influences_;
}

void AIAnalysisPass::computeGlobalRiskIndex() {
    if (risk_propagation_map_.empty()) {
        context_.global_risk_index = 0.0f;
        return;
    }
    
    float sum_risk = 0.0f;
    float max_risk = 0.0f;
    
    for (const auto& cell : risk_propagation_map_) {
        sum_risk += cell.risk_value;
        max_risk = std::max(max_risk, cell.risk_value);
    }
    
    float avg_risk = sum_risk / risk_propagation_map_.size();
    context_.global_risk_index = 0.6f * avg_risk + 0.4f * max_risk;
}

void AIAnalysisPass::computeSystemCongestion() {
    if (risk_propagation_map_.empty()) {
        context_.system_congestion = 0.0f;
        return;
    }
    
    uint32_t high_risk_count = 0;
    for (const auto& cell : risk_propagation_map_) {
        if (cell.risk_value > 0.7f) {
            high_risk_count++;
        }
    }
    
    context_.system_congestion = static_cast<float>(high_risk_count) / risk_propagation_map_.size();
}

void AIAnalysisPass::computeAnomalyScore() {
    float risk_variance = 0.0f;
    float mean_risk = context_.global_risk_index;
    
    for (const auto& cell : risk_propagation_map_) {
        float diff = cell.risk_value - mean_risk;
        risk_variance += diff * diff;
    }
    
    if (!risk_propagation_map_.empty()) {
        risk_variance /= risk_propagation_map_.size();
    }
    
    context_.anomaly_score = std::min(1.0f, std::sqrt(risk_variance) * 2.0f);
}

void AIAnalysisPass::propagateRisk() {
    for (auto& cell : risk_propagation_map_) {
        cell.propagation_risk = cell.risk_value * RISK_DECAY_FACTOR;
    }
    
    for (size_t i = 0; i < risk_propagation_map_.size(); ++i) {
        for (size_t j = 0; j < risk_propagation_map_.size(); ++j) {
            if (i == j) continue;
            
            const auto& source = risk_propagation_map_[i];
            auto& target = risk_propagation_map_[j];
            
            float dx = static_cast<float>(source.x - target.x);
            float dy = static_cast<float>(source.y - target.y);
            float distance = std::sqrt(dx * dx + dy * dy);
            
            if (distance < 5.0f) {
                float propagation = source.risk_value * std::exp(-distance * 0.5f);
                target.propagation_risk = std::max(target.propagation_risk, propagation);
            }
        }
    }
}

void AIAnalysisPass::computeAssetInfluences() {
    for (const auto& cell : risk_propagation_map_) {
        if (cell.risk_value > ANOMALY_THRESHOLD) {
            AssetInfluence influence;
            influence.asset_id = cell.source_asset_count;
            influence.influence_radius = cell.risk_value * 10.0f;
            influence.risk_contribution = cell.risk_value;
            
            for (const auto& otherCell : risk_propagation_map_) {
                float dx = static_cast<float>(cell.x - otherCell.x);
                float dy = static_cast<float>(cell.y - otherCell.y);
                float distance = std::sqrt(dx * dx + dy * dy);
                
                if (distance < influence.influence_radius && otherCell.risk_value > ANOMALY_THRESHOLD) {
                    influence.affected_zone_ids.push_back(otherCell.source_asset_count);
                }
            }
            
            asset_influences_.push_back(influence);
        }
    }
}