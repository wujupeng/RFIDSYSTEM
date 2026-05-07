#include "confidence_engine.h"

ConfidenceEngine::ConfidenceEngine() {
    sources_["rssi"] = {"rssi", 0.3, 0.5, true};
    sources_["phase"] = {"phase", 0.4, 0.8, true};
    sources_["aoa"] = {"aoa", 0.3, 0.7, true};
}

ConfidenceEngine& ConfidenceEngine::instance() {
    static ConfidenceEngine instance;
    return instance;
}

void ConfidenceEngine::updateSource(const std::string& name, double confidence, bool trusted) {
    auto it = sources_.find(name);
    if (it != sources_.end()) {
        it->second.confidence = confidence;
        it->second.trusted = trusted;
    }
}

double ConfidenceEngine::getOverallConfidence() {
    double weighted_sum = 0.0;
    double weight_sum = 0.0;
    
    for (const auto& pair : sources_) {
        if (pair.second.trusted) {
            weighted_sum += pair.second.weight * pair.second.confidence;
            weight_sum += pair.second.weight;
        }
    }
    
    return weight_sum > 0 ? weighted_sum / weight_sum : 0.0;
}

void ConfidenceEngine::propagateConfidence() {
    double overall = getOverallConfidence();
    
    for (auto& pair : sources_) {
        if (!pair.second.trusted) {
            pair.second.confidence = std::min(1.0, pair.second.confidence + overall * 0.1);
        }
    }
}

void ConfidenceEngine::decay() {
    for (auto& pair : sources_) {
        pair.second.confidence = std::max(0.1, pair.second.confidence * 0.99);
    }
}

std::vector<ConfidenceSource> ConfidenceEngine::getSources() {
    std::vector<ConfidenceSource> result;
    for (const auto& pair : sources_) {
        result.push_back(pair.second);
    }
    return result;
}