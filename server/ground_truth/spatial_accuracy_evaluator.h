#pragma once

#include <cstdint>
#include <vector>
#include <map>

namespace ground_truth {

struct PositionEstimate {
    uint64_t tag_id;
    float estimated_x;
    float estimated_y;
    float estimated_z;
    float confidence;
    uint64_t timestamp;
};

struct AccuracyResult {
    uint64_t anchor_id;
    float error_m;
    float error_x;
    float error_y;
    float error_z;
    float rmse;
    float max_error;
    float min_error;
    uint64_t sample_count;
    uint64_t timestamp;
};

class SpatialAccuracyEvaluator {
public:
    static SpatialAccuracyEvaluator& instance();
    
    void addEstimate(const PositionEstimate& estimate);
    
    AccuracyResult evaluateAnchor(uint64_t anchor_id);
    
    AccuracyResult evaluateAllAnchors();
    
    std::map<uint64_t, AccuracyResult> evaluateAnchors();
    
    float getOverallRMSE() const;
    
    float getMaxError() const;
    
    float getMinError() const;
    
    void reset();
    
    void setWindowSize(size_t window_size);
    
private:
    SpatialAccuracyEvaluator() = default;
    
    std::vector<PositionEstimate> recent_estimates_;
    size_t window_size_ = 100;
};

} // namespace ground_truth