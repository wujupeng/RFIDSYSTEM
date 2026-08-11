#pragma once

#include "factory_map.h"
#include "heatmap_builder.h"
#include "trail_collector.h"
#include "node_status_encoder.h"
#include "../spatial_graph/spatial_graph_builder.h"
#include "../spatial_estimate/spatial_estimate.h"
#include <vector>
#include <string>
#include <map>

namespace pa::spatial_truth {

struct FactoryMapInput {
    std::vector<ReaderInfo> readers;
    std::vector<TagAssetBinding> bindings;
    std::vector<TagEstimate> tag_estimates;
    std::vector<ZoneInfo> zones;
    std::vector<RFObservation> rf_observations;
    std::vector<RFEnvironmentData> rf_env_data;
    std::vector<AoAObservation> aoa_observations;
    std::map<std::string, SpatialEstimate> estimates;
    double min_x = 0.0, max_x = 100.0, min_y = 0.0, max_y = 100.0;
    double heatmap_resolution = 1.0;
};

class FactoryMapGenerator {
public:
    FactoryMap generate(const FactoryMapInput& input) const;
    FactoryMap generateByZone(const FactoryMapInput& input, const std::string& zone_id) const;

private:
    SpatialGraphBuilder graph_builder_;
    HeatmapBuilder heatmap_builder_;
    AoAVectorBuilder aoa_builder_;
    NodeStatusEncoder status_encoder_;
};

class FactoryMapPartitioner {
public:
    std::map<std::string, FactoryMap> partitionByZone(const FactoryMap& factory_map) const;
    FactoryMap getViewport(const FactoryMap& factory_map, double min_x, double max_x,
                           double min_y, double max_y) const;
};

} // namespace pa::spatial_truth