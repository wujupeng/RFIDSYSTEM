#pragma once

#include <string>
#include <vector>
#include <map>
#include <cstdint>

namespace pa::spatial_truth {

enum class SpatialNodeType {
    READER,
    TAG,
    ASSET,
    ZONE,
    WORKSHOP,
};

enum class SpatialEdgeType {
    READER_TAG,
    TAG_ASSET,
    ASSET_ZONE,
    ZONE_WORKSHOP,
    READER_READER,
};

enum class EdgeSource {
    READER_TOPOLOGY,
    SPATIAL_TOPOLOGY,
    RF_ENVIRONMENT,
};

struct SpatialGraphNode {
    std::string node_id;
    SpatialNodeType node_type = SpatialNodeType::READER;
    std::map<std::string, std::string> attributes;
};

struct SpatialGraphEdge {
    std::string from_node;
    std::string to_node;
    SpatialEdgeType edge_type = SpatialEdgeType::READER_TAG;
    EdgeSource source = EdgeSource::READER_TOPOLOGY;
    double weight = 1.0;
};

struct SpatialGraph {
    std::vector<SpatialGraphNode> nodes;
    std::vector<SpatialGraphEdge> edges;
    uint64_t timestamp = 0;
};

inline const char* nodeTypeToString(SpatialNodeType t) {
    switch (t) {
        case SpatialNodeType::READER:   return "reader";
        case SpatialNodeType::TAG:      return "tag";
        case SpatialNodeType::ASSET:    return "asset";
        case SpatialNodeType::ZONE:     return "zone";
        case SpatialNodeType::WORKSHOP: return "workshop";
    }
    return "unknown";
}

inline const char* edgeTypeToString(SpatialEdgeType t) {
    switch (t) {
        case SpatialEdgeType::READER_TAG:    return "reader_tag";
        case SpatialEdgeType::TAG_ASSET:     return "tag_asset";
        case SpatialEdgeType::ASSET_ZONE:    return "asset_zone";
        case SpatialEdgeType::ZONE_WORKSHOP: return "zone_workshop";
        case SpatialEdgeType::READER_READER: return "reader_reader";
    }
    return "unknown";
}

} // namespace pa::spatial_truth