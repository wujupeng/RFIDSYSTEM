#include "spatial_graph_projector.h"

namespace pa::spatial_truth {

TopologyGraph SpatialGraphProjector::toTopologyGraph(const SpatialGraph& spatial_graph) const {
    TopologyGraph result;

    for (const auto& node : spatial_graph.nodes) {
        if (node.node_type != SpatialNodeType::READER) continue;

        TopologyNode tn;
        tn.reader_id = node.node_id;
        auto it = node.attributes.find("x");
        if (it != node.attributes.end()) tn.x = std::stod(it->second);
        it = node.attributes.find("y");
        if (it != node.attributes.end()) tn.y = std::stod(it->second);
        it = node.attributes.find("coverage_radius");
        if (it != node.attributes.end()) tn.coverage_radius = std::stod(it->second);
        it = node.attributes.find("name");
        if (it != node.attributes.end()) tn.name = it->second;
        tn.online = true;
        result.nodes.push_back(tn);
    }

    for (const auto& edge : spatial_graph.edges) {
        if (edge.edge_type != SpatialEdgeType::READER_READER) continue;

        TopologyEdge te;
        te.from = edge.from_node;
        te.to = edge.to_node;
        te.strength = edge.weight;
        result.edges.push_back(te);
    }

    return result;
}

} // namespace pa::spatial_truth