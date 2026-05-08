#pragma once

#include <cstdint>
#include <vector>
#include <map>

namespace spatial_drift {

struct DriftMetric {
    uint64_t timestamp;
    float rmse_trend;
    float error_increase_rate;
    float spatial_bias_x;
    float spatial_bias_y;
    float confidence_degradation;
    int consecutive_high_error_count;
};

struct DriftAlert {
    uint64_t alert_id;
    uint64_t timestamp;
    float severity;
    std::string message;
    bool acknowledged;
};

class DriftMetricTracker {
public:
    static DriftMetricTracker& instance();
    
    void addMetric(float rmse, float error_rate, float bias_x, float bias_y, float confidence);
    
    DriftMetric getLatestMetric() const;
    
    std::vector<DriftMetric> getMetricsSince(uint64_t timestamp) const;
    
    float getAverageRMSE() const;
    
    float getMaxErrorRate() const;
    
    bool isDriftDetected() const;
    
    void reset();
    
private:
    DriftMetricTracker() = default;
    
    std::vector<DriftMetric> metrics_;
    size_t max_metrics_ = 1000;
};

} // namespace spatial_drift