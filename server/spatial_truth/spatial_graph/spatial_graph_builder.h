#pragma once

#include "spatial_graph.h"
#include "reader_topology_auto_discovery.h"
#include "tag_asset_binding_sync.h"
#include <vector>

namespace pa::spatial_truth {

struct RFObservation {
    std::string reader_id;
    std::string tag_id;
    double rssi = 0.0;
};

class SpatialGraphBuilder {
public:
    SpatialGraph build(
        const std::vector<ReaderInfo>& readers,
        const std::vector<TagAssetBinding>& bindings,
        const std::vector<TagEstimate>& tag_estimates,
        const std::vector<ZoneInfo>& zones,
        const std::vector<RFObservation>& rf_observations
    ) const;

private:
    ReaderTopologyAutoDiscovery topology_discovery_;
    TagAssetBindingSync binding_sync_;
    SpatialOwnershipResolver ownership_resolver_;

    void removeDanglingReferences(SpatialGraph& graph) const;
};

} // namespace pa::spatial_truth