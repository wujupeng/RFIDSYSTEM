#include "spatial_estimate_projector.h"

namespace pa::spatial_truth {

TagPosition SpatialEstimateProjector::toTagPosition(const SpatialEstimate& estimate, uint64_t timestamp) const {
    TagPosition tp;
    tp.x = estimate.x;
    tp.y = estimate.y;
    tp.confidence = estimate.confidence;
    tp.velocity_x = 0.0;
    tp.velocity_y = 0.0;
    tp.timestamp = timestamp;
    return tp;
}

std::vector<TagPosition> SpatialEstimateProjector::toTagPositionList(const std::vector<SpatialEstimate>& estimates, uint64_t timestamp) const {
    std::vector<TagPosition> result;
    result.reserve(estimates.size());
    for (const auto& est : estimates) {
        result.push_back(toTagPosition(est, timestamp));
    }
    return result;
}

} // namespace pa::spatial_truth