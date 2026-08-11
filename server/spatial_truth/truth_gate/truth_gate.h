#pragma once

#include "../spatial_estimate/spatial_estimate.h"
#include "../ground_truth/ground_truth_point.h"
#include <string>
#include <cstdint>
#include <optional>

namespace pa::spatial_truth {

struct TruthGateResult {
    bool pass = false;
    bool soft = false;
    double error_value = 0.0;
    double threshold = 0.0;
    std::string tag_id;
    uint64_t frame_id = 0;
    std::string reason;
};

class TruthGate {
public:
    TruthGateResult evaluate(const SpatialEstimate& estimate, uint64_t frame_id) const;

    void setDriftThreshold(double threshold) { drift_threshold_ = threshold; }
    void setQueryTimeoutMs(uint64_t timeout) { query_timeout_ms_ = timeout; }

private:
    double drift_threshold_ = 0.20;
    uint64_t query_timeout_ms_ = 10;

    std::optional<GroundTruthPoint> queryGroundTruth(const std::string& tag_id, uint64_t timestamp) const;
};

class SoftTruthGate {
public:
    TruthGateResult evaluate(const SpatialEstimate& estimate) const;

    void setConfidenceThreshold(double threshold) { confidence_threshold_ = threshold; }

private:
    double confidence_threshold_ = 0.7;
};

} // namespace pa::spatial_truth