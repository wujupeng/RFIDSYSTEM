#include "reader_topology_auto_discovery.h"
#include <cmath>
#include <algorithm>

namespace pa::spatial_truth {

std::vector<SpatialGraphNode> ReaderTopologyAutoDiscovery::discover(const std::vector<ReaderInfo>& readers) const {
    std::vector<SpatialGraphNode> nodes;
    for (const auto& r : readers) {
        if (!r.online) continue;
        SpatialGraphNode node;
        node.node_id = r.reader_id;
        node.node_type = SpatialNodeType::READER;
        node.attributes["x"] = std::to_string(r.x);
        node.attributes["y"] = std::to_string(r.y);
        node.attributes["coverage_radius"] = std::to_string(r.coverage_radius);
        node.attributes["antenna_count"] = std::to_string(r.antenna_count);
        node.attributes["name"] = r.name;
        nodes.push_back(node);
    }
    return nodes;
}

std::vector<SpatialGraphEdge> ReaderTopologyAutoDiscovery::discoverEdges(const std::vector<ReaderInfo>& readers) const {
    std::vector<SpatialGraphEdge> edges;
    for (size_t i = 0; i < readers.size(); ++i) {
        if (!readers[i].online) continue;
        for (size_t j = i + 1; j < readers.size(); ++j) {
            if (!readers[j].online) continue;
            double dx = readers[i].x - readers[j].x;
            double dy = readers[i].y - readers[j].y;
            double dist = std::sqrt(dx * dx + dy * dy);
            double combined_coverage = readers[i].coverage_radius + readers[j].coverage_radius;
            if (dist <= combined_coverage) {
                SpatialGraphEdge edge;
                edge.from_node = readers[i].reader_id;
                edge.to_node = readers[j].reader_id;
                edge.edge_type = SpatialEdgeType::READER_READER;
                edge.source = EdgeSource::READER_TOPOLOGY;
                edge.weight = 1.0 / (1.0 + dist);
                edges.push_back(edge);
            }
        }
    }
    return edges;
}

SpatialGraph ReaderTopologyAutoDiscovery::removeOfflineReader(const SpatialGraph& graph, const std::string& reader_id) const {
    SpatialGraph result;
    result.timestamp = graph.timestamp;

    for (const auto& node : graph.nodes) {
        if (node.node_id == reader_id && node.node_type == SpatialNodeType::READER) continue;
        result.nodes.push_back(node);
    }

    for (const auto& edge : graph.edges) {
        if (edge.from_node == reader_id || edge.to_node == reader_id) continue;
        result.edges.push_back(edge);
    }

    return result;
}

} // namespace pa::spatial_truth