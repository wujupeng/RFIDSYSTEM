#pragma once

#include "../spatial_graph/spatial_graph.h"
#include "../spatial_estimate/spatial_estimate.h"
#include <vector>
#include <string>
#include <map>
#include <cstdint>

namespace pa::spatial_truth {

struct Heatmap {
    std::vector<std::vector<double>> grid;
    double resolution = 1.0;
};

struct AoAVector {
    std::string reader_id;
    double origin_x = 0.0;
    double origin_y = 0.0;
    double direction_x = 0.0;
    double direction_y = 0.0;
};

struct Trail {
    std::string tag_id;
    std::vector<Point3D> points;
    std::vector<uint64_t> timestamps;
};

struct Flow {
    std::string zone_id;
    int in_count = 0;
    int out_count = 0;
    uint64_t timestamp = 0;
};

enum class NodeStatus {
    NORMAL,
    UNCERTAIN,
    RISK,
};

struct FactoryMap {
    SpatialGraph spatial_graph;
    Heatmap heatmap;
    std::vector<AoAVector> aoa_vectors;
    std::vector<Trail> trails;
    std::vector<Flow> flows;
    std::map<std::string, NodeStatus> node_status;
    uint64_t update_timestamp = 0;
};

} // namespace pa::spatial_truth