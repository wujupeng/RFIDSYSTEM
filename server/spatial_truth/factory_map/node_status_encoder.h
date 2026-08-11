#pragma once

#include "factory_map.h"
#include "../spatial_estimate/spatial_estimate.h"
#include <vector>
#include <string>
#include <map>

namespace pa::spatial_truth {

class NodeStatusEncoder {
public:
    NodeStatus encode(const SpatialEstimate& estimate) const;
    std::map<std::string, NodeStatus> encodeAll(const std::map<std::string, SpatialEstimate>& estimates) const;
};

} // namespace pa::spatial_truth