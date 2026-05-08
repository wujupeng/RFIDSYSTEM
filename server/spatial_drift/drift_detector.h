#pragma once

#include <cstdint>
#include <vector>
#include <map>
#include "drift_metric.h"

namespace spatial_drift {

enum class DriftLevel {
    NONE,
    WARNING,
    CRITICAL,
    EMERGENCY
};

struct DriftDetectionResult {
    bool drift_detected;
    DriftLevel level;
    float drift_magnitude;
    std::string cause;
    uint64_t timestamp;
};

class DriftDetector {
public:
    static DriftDetector& instance();
    
    void initialize();
    
    DriftDetectionResult detect();
    
    DriftLevel getCurrentLevel() const;
    
    void setThresholds(float warning_threshold, float critical_threshold, float emergency_threshold);
    
    std::vector<DriftAlert> getAlerts();
    
    void acknowledgeAlert(uint64_t alert_id);
    
    void reset();
    
private:
    DriftDetector() = default;
    
    void evaluateMetrics();
    
    void generateAlert(DriftLevel level, const std::string& cause);
    
    DriftLevel current_level_ = DriftLevel::NONE;
    float warning_threshold_ = 0.3f;
    float critical_threshold_ = 0.5f;
    float emergency_threshold_ = 1.0f;
    std::vector<DriftAlert> alerts_;
};

} // namespace spatial_drift