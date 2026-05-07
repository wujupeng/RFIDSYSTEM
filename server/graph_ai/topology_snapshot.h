#pragma once

#include "../topology/topology_types.h"

struct TopologySnapshot {
    uint64_t timestamp;
    
    std::vector<TopologyNode> nodes;
    std::vector<TopologyEdge> edges;
    
    TopologySnapshot() : timestamp(0) {}
};