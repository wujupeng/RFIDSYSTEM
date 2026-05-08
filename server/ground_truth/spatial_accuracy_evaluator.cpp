#include "spatial_accuracy_evaluator.h"
#include "truth_anchor_manager.h"

namespace ground_truth {

SpatialAccuracyEvaluator& SpatialAccuracyEvaluator::instance() {
    static SpatialAccuracyEvaluator evaluator;
    return evaluator;
}

void SpatialAccuracyEvaluator::addEstimate(const PositionEstimate& estimate) {
    recent_estimates_.push_back(estimate);
    if (recent_estimates_.size() > window_size_) {
        recent_estimates_.erase(recent_estimates_.begin());
    }
}

AccuracyResult SpatialAccuracyEvaluator::evaluateAnchor(uint64_t anchor_id) {
    const TruthAnchor* anchor = TruthAnchorManager::instance().getAnchor(anchor_id);
    if (!anchor || !anchor->active) {
        return {};
    }
    
    AccuracyResult result;
    result.anchor_id = anchor_id;
    result.sample_count = 0;
    result.rmse = 0.0f;
    result.max_error = 0.0f;
    result.min_error = FLT_MAX;
    
    float sum_squared_error = 0.0f;
    
    for (const auto& estimate : recent_estimates_) {
        float dx = estimate.estimated_x - anchor->x;
        float dy = estimate.estimated_y - anchor->y;
        float dz = estimate.estimated_z - anchor->z;
        float error = std::sqrt(dx * dx + dy * dy + dz * dz);
        
        result.error_x += dx;
        result.error_y += dy;
        result.error_z += dz;
        sum_squared_error += error * error;
        result.max_error = std::max(result.max_error, error);
        result.min_error = std::min(result.min_error, error);
        result.sample_count++;
    }
    
    if (result.sample_count > 0) {
        result.error_x /= result.sample_count;
        result.error_y /= result.sample_count;
        result.error_z /= result.sample_count;
        result.error_m = std::sqrt(result.error_x * result.error_x + 
                                   result.error_y * result.error_y + 
                                   result.error_z * result.error_z);
        result.rmse = std::sqrt(sum_squared_error / result.sample_count);
    }
    
    return result;
}

AccuracyResult SpatialAccuracyEvaluator::evaluateAllAnchors() {
    AccuracyResult overall;
    overall.sample_count = 0;
    overall.rmse = 0.0f;
    overall.max_error = 0.0f;
    overall.min_error = FLT_MAX;
    
    float total_rmse = 0.0f;
    int count = 0;
    
    auto anchors = TruthAnchorManager::instance().getActiveAnchors();
    for (const auto& anchor : anchors) {
        AccuracyResult result = evaluateAnchor(anchor.anchor_id);
        if (result.sample_count > 0) {
            total_rmse += result.rmse;
            overall.max_error = std::max(overall.max_error, result.max_error);
            overall.min_error = std::min(overall.min_error, result.min_error);
            overall.sample_count += result.sample_count;
            count++;
        }
    }
    
    if (count > 0) {
        overall.rmse = total_rmse / count;
    }
    
    return overall;
}

std::map<uint64_t, AccuracyResult> SpatialAccuracyEvaluator::evaluateAnchors() {
    std::map<uint64_t, AccuracyResult> results;
    auto anchors = TruthAnchorManager::instance().getActiveAnchors();
    for (const auto& anchor : anchors) {
        results[anchor.anchor_id] = evaluateAnchor(anchor.anchor_id);
    }
    return results;
}

float SpatialAccuracyEvaluator::getOverallRMSE() const {
    return evaluateAllAnchors().rmse;
}

float SpatialAccuracyEvaluator::getMaxError() const {
    return evaluateAllAnchors().max_error;
}

float SpatialAccuracyEvaluator::getMinError() const {
    return evaluateAllAnchors().min_error;
}

void SpatialAccuracyEvaluator::reset() {
    recent_estimates_.clear();
}

void SpatialAccuracyEvaluator::setWindowSize(size_t window_size) {
    window_size_ = window_size;
}

} // namespace ground_truth