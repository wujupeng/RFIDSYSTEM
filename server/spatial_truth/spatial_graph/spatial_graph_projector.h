#pragma once

#include "spatial_graph.h"
#include <vector>
#include <string>

namespace pa::spatial_truth {

struct TopologyNode {
    std::string reader_id;
    double x = 0.0;
    double y = 0.0;
    double coverage_radius = 10.0;
    double load = 0.0;
    bool online = true;
    std::string name;
};

struct TopologyEdge {
    std::string from;
    std::string to;
    double strength = 0.0;
};

struct TopologyGraph {
    std::vector<TopologyNode> nodes;
    std::vector<TopologyEdge> edges;
};

class SpatialGraphProjector {
public:
    TopologyGraph toTopologyGraph(const SpatialGraph& spatial_graph) const;
};

} // namespace pa::spatial_truth