#pragma once

#include "spatial_graph.h"
#include <vector>

namespace pa::spatial_truth {

struct ReaderInfo {
    std::string reader_id;
    double x = 0.0;
    double y = 0.0;
    double coverage_radius = 10.0;
    int antenna_count = 1;
    bool online = true;
    std::string name;
};

class ReaderTopologyAutoDiscovery {
public:
    std::vector<SpatialGraphNode> discover(const std::vector<ReaderInfo>& readers) const;
    std::vector<SpatialGraphEdge> discoverEdges(const std::vector<ReaderInfo>& readers) const;
    SpatialGraph removeOfflineReader(const SpatialGraph& graph, const std::string& reader_id) const;
};

} // namespace pa::spatial_truth