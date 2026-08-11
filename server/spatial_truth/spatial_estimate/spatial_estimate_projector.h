#pragma once

#include "spatial_estimate.h"
#include <vector>

namespace pa::spatial_truth {

class SpatialEstimateProjector {
public:
    TagPosition toTagPosition(const SpatialEstimate& estimate, uint64_t timestamp = 0) const;
    std::vector<TagPosition> toTagPositionList(const std::vector<SpatialEstimate>& estimates, uint64_t timestamp = 0) const;
};

} // namespace pa::spatial_truth