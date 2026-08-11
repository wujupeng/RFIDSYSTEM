#pragma once

#include "spatial_graph.h"
#include <vector>
#include <string>

namespace pa::spatial_truth {

struct TagAssetBinding {
    std::string tag_id;
    std::string asset_id;
};

class TagAssetBindingSync {
public:
    std::vector<SpatialGraphEdge> sync(const std::vector<TagAssetBinding>& bindings) const;
    std::vector<SpatialGraphNode> buildNodes(const std::vector<TagAssetBinding>& bindings) const;
};

struct ZoneInfo {
    std::string zone_id;
    double min_x = 0.0;
    double max_x = 0.0;
    double min_y = 0.0;
    double max_y = 0.0;
};

struct TagEstimate {
    std::string tag_id;
    double x = 0.0;
    double y = 0.0;
};

class SpatialOwnershipResolver {
public:
    std::vector<SpatialGraphEdge> resolve(const std::vector<TagEstimate>& tag_estimates,
                                          const std::vector<ZoneInfo>& zones) const;
};

} // namespace pa::spatial_truth