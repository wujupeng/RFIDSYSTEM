#include "node_status_encoder.h"

namespace pa::spatial_truth {

NodeStatus NodeStatusEncoder::encode(const SpatialEstimate& estimate) const {
    if (estimate.trusted) return NodeStatus::NORMAL;
    if (estimate.confidence > 0.0) return NodeStatus::UNCERTAIN;
    return NodeStatus::RISK;
}

std::map<std::string, NodeStatus> NodeStatusEncoder::encodeAll(
    const std::map<std::string, SpatialEstimate>& estimates
) const {
    std::map<std::string, NodeStatus> result;
    for (const auto& [id, est] : estimates) {
        result[id] = encode(est);
    }
    return result;
}

} // namespace pa::spatial_truth