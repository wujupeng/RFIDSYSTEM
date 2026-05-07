#pragma once

#include <cstdint>
#include <vector>
#include <string>

struct TopologyNode {
    uint64_t reader_id;
    float x, y;
    float coverage_radius;
    float load;
    bool online;
    std::string name;
    
    TopologyNode() 
        : reader_id(0), x(0), y(0), coverage_radius(0), 
          load(0), online(false) {}
};

struct TopologyEdge {
    uint64_t from;
    uint64_t to;
    float strength;
    float transition_prob;
    float overlap;
    float correlation;
    
    TopologyEdge() 
        : from(0), to(0), strength(0), transition_prob(0), 
          overlap(0), correlation(0) {}
};

struct TopologyGraph {
    std::vector<TopologyNode> nodes;
    std::vector<TopologyEdge> edges;
    uint64_t timestamp;
};

struct RFIDEvent {
    uint64_t tag_id;
    uint64_t reader_id;
    float rssi;
    uint64_t timestamp;
    float x, y;
};