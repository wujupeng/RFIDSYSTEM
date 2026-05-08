#include "drift_detector.h"

namespace spatial_drift {

DriftDetector& DriftDetector::instance() {
    static DriftDetector detector;
    return detector;
}

void DriftDetector::initialize() {
    current_level_ = DriftLevel::NONE;
}

DriftDetectionResult DriftDetector::detect() {
    evaluateMetrics();
    
    DriftDetectionResult result;
    result.drift_detected = current_level_ != DriftLevel::NONE;
    result.level = current_level_;
    result.drift_magnitude = DriftMetricTracker::instance().getAverageRMSE();
    result.timestamp = 0;
    
    if (current_level_ == DriftLevel::WARNING) {
        result.cause = "RMSE exceeding warning threshold";
    } else if (current_level_ == DriftLevel::CRITICAL) {
        result.cause = "RMSE exceeding critical threshold";
    } else if (current_level_ == DriftLevel::EMERGENCY) {
        result.cause = "RMSE exceeding emergency threshold";
    }
    
    return result;
}

DriftLevel DriftDetector::getCurrentLevel() const {
    return current_level_;
}

void DriftDetector::setThresholds(float warning_threshold, float critical_threshold, float emergency_threshold) {
    warning_threshold_ = warning_threshold;
    critical_threshold_ = critical_threshold;
    emergency_threshold_ = emergency_threshold;
}

std::vector<DriftAlert> DriftDetector::getAlerts() {
    return alerts_;
}

void DriftDetector::acknowledgeAlert(uint64_t alert_id) {
    for (auto& alert : alerts_) {
        if (alert.alert_id == alert_id) {
            alert.acknowledged = true;
            break;
        }
    }
}

void DriftDetector::reset() {
    current_level_ = DriftLevel::NONE;
    alerts_.clear();
}

void DriftDetector::evaluateMetrics() {
    float avg_rmse = DriftMetricTracker::instance().getAverageRMSE();
    
    if (avg_rmse >= emergency_threshold_) {
        current_level_ = DriftLevel::EMERGENCY;
        generateAlert(DriftLevel::EMERGENCY, "Emergency: RMSE = " + std::to_string(avg_rmse));
    } else if (avg_rmse >= critical_threshold_) {
        current_level_ = DriftLevel::CRITICAL;
        generateAlert(DriftLevel::CRITICAL, "Critical: RMSE = " + std::to_string(avg_rmse));
    } else if (avg_rmse >= warning_threshold_) {
        current_level_ = DriftLevel::WARNING;
        generateAlert(DriftLevel::WARNING, "Warning: RMSE = " + std::to_string(avg_rmse));
    } else {
        current_level_ = DriftLevel::NONE;
    }
}

void DriftDetector::generateAlert(DriftLevel level, const std::string& cause) {
    DriftAlert alert;
    alert.alert_id = static_cast<uint64_t>(alerts_.size()) + 1;
    alert.timestamp = 0;
    alert.severity = static_cast<float>(level);
    alert.message = cause;
    alert.acknowledged = false;
    
    alerts_.push_back(alert);
}

} // namespace spatial_drift