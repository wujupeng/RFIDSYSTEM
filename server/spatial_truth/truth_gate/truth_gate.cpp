#include "truth_gate.h"
#include "../ground_truth/ground_truth_store.h"
#include <cmath>
#include <chrono>

namespace pa::spatial_truth {

std::optional<GroundTruthPoint> TruthGate::queryGroundTruth(const std::string& tag_id, uint64_t timestamp) const {
    auto points = GroundTruthStore::instance().listByTagId(tag_id);
    if (points.empty()) return std::nullopt;

    const GroundTruthPoint* best = nullptr;
    uint64_t min_diff = UINT64_MAX;
    for (const auto& p : points) {
        uint64_t diff = (p.timestamp > timestamp) ? (p.timestamp - timestamp) : (timestamp - p.timestamp);
        if (diff < min_diff) {
            min_diff = diff;
            best = &p;
        }
    }
    if (best) return *best;
    return std::nullopt;
}

TruthGateResult TruthGate::evaluate(const SpatialEstimate& estimate, uint64_t frame_id) const {
    TruthGateResult result;
    result.tag_id = "";
    result.frame_id = frame_id;
    result.threshold = drift_threshold_;

    if (estimate.calibration_version == 0) {
        result.soft = true;
        result.pass = (estimate.confidence >= 0.7);
        result.reason = "未标定，软判定";
        return result;
    }

    auto gt = queryGroundTruth("", estimate.source_hash);
    if (!gt) {
        result.soft = true;
        result.pass = (estimate.confidence >= 0.7);
        result.reason = "无 Ground Truth，软判定";
        return result;
    }

    double dx = estimate.x - gt->x;
    double dy = estimate.y - gt->y;
    double dz = estimate.z - gt->z;
    double error = std::sqrt(dx * dx + dy * dy + dz * dz);

    result.error_value = error;
    result.pass = (error <= drift_threshold_);
    if (!result.pass) {
        result.reason = "位置漂移超过阈值";
    } else {
        result.reason = "通过";
    }

    return result;
}

TruthGateResult SoftTruthGate::evaluate(const SpatialEstimate& estimate) const {
    TruthGateResult result;
    result.soft = true;

    if (estimate.calibration_version == 0) {
        result.pass = false;
        result.reason = "未标定，软判定";
        return result;
    }

    result.pass = (estimate.confidence >= confidence_threshold_);
    result.reason = result.pass ? "软判定通过" : "软判定不通过";
    return result;
}

} // namespace pa::spatial_truth