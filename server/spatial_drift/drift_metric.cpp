#include "drift_metric.h"

namespace spatial_drift {

DriftMetricTracker& DriftMetricTracker::instance() {
    static DriftMetricTracker tracker;
    return tracker;
}

void DriftMetricTracker::addMetric(float rmse, float error_rate, float bias_x, float bias_y, float confidence) {
    DriftMetric metric;
    metric.timestamp = 0;
    metric.rmse_trend = rmse;
    metric.error_increase_rate = error_rate;
    metric.spatial_bias_x = bias_x;
    metric.spatial_bias_y = bias_y;
    metric.confidence_degradation = confidence;
    metric.consecutive_high_error_count = 0;
    
    metrics_.push_back(metric);
    if (metrics_.size() > max_metrics_) {
        metrics_.erase(metrics_.begin());
    }
}

DriftMetric DriftMetricTracker::getLatestMetric() const {
    if (metrics_.empty()) {
        return {};
    }
    return metrics_.back();
}

std::vector<DriftMetric> DriftMetricTracker::getMetricsSince(uint64_t timestamp) const {
    std::vector<DriftMetric> result;
    for (const auto& metric : metrics_) {
        if (metric.timestamp >= timestamp) {
            result.push_back(metric);
        }
    }
    return result;
}

float DriftMetricTracker::getAverageRMSE() const {
    if (metrics_.empty()) {
        return 0.0f;
    }
    float sum = 0.0f;
    for (const auto& metric : metrics_) {
        sum += metric.rmse_trend;
    }
    return sum / metrics_.size();
}

float DriftMetricTracker::getMaxErrorRate() const {
    if (metrics_.empty()) {
        return 0.0f;
    }
    float max_rate = 0.0f;
    for (const auto& metric : metrics_) {
        max_rate = std::max(max_rate, metric.error_increase_rate);
    }
    return max_rate;
}

bool DriftMetricTracker::isDriftDetected() const {
    if (metrics_.size() < 10) {
        return false;
    }
    
    float recent_avg = 0.0f;
    float older_avg = 0.0f;
    int recent_count = metrics_.size() / 2;
    
    for (size_t i = 0; i < metrics_.size(); ++i) {
        if (i >= metrics_.size() - recent_count) {
            recent_avg += metrics_[i].rmse_trend;
        } else {
            older_avg += metrics_[i].rmse_trend;
        }
    }
    
    recent_avg /= recent_count;
    older_avg /= (metrics_.size() - recent_count);
    
    return recent_avg > older_avg * 1.5f;
}

void DriftMetricTracker::reset() {
    metrics_.clear();
}

} // namespace spatial_drift