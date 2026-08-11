#include "spatial_graph_builder.h"
#include <chrono>
#include <unordered_set>
#include <algorithm>

namespace pa::spatial_truth {

void SpatialGraphBuilder::removeDanglingReferences(SpatialGraph& graph) const {
    std::unordered_set<std::string> node_ids;
    for (const auto& node : graph.nodes) {
        node_ids.insert(node.node_id);
    }

    std::vector<SpatialGraphEdge> valid_edges;
    for (const auto& edge : graph.edges) {
        if (node_ids.count(edge.from_node) && node_ids.count(edge.to_node)) {
            valid_edges.push_back(edge);
        }
    }
    graph.edges = std::move(valid_edges);
}

SpatialGraph SpatialGraphBuilder::build(
    const std::vector<ReaderInfo>& readers,
    const std::vector<TagAssetBinding>& bindings,
    const std::vector<TagEstimate>& tag_estimates,
    const std::vector<ZoneInfo>& zones,
    const std::vector<RFObservation>& rf_observations
) const {
    SpatialGraph graph;

    auto reader_nodes = topology_discovery_.discover(readers);
    auto reader_edges = topology_discovery_.discoverEdges(readers);

    for (const auto& node : reader_nodes) graph.nodes.push_back(node);
    for (const auto& edge : reader_edges) graph.edges.push_back(edge);

    auto binding_nodes = binding_sync_.buildNodes(bindings);
    auto binding_edges = binding_sync_.sync(bindings);

    for (const auto& node : binding_nodes) graph.nodes.push_back(node);
    for (const auto& edge : binding_edges) graph.edges.push_back(edge);

    auto ownership_edges = ownership_resolver_.resolve(tag_estimates, zones);
    for (const auto& edge : ownership_edges) graph.edges.push_back(edge);

    for (const auto& zone : zones) {
        SpatialGraphNode zone_node;
        zone_node.node_id = zone.zone_id;
        zone_node.node_type = SpatialNodeType::ZONE;
        zone_node.attributes["min_x"] = std::to_string(zone.min_x);
        zone_node.attributes["max_x"] = std::to_string(zone.max_x);
        zone_node.attributes["min_y"] = std::to_string(zone.min_y);
        zone_node.attributes["max_y"] = std::to_string(zone.max_y);
        graph.nodes.push_back(zone_node);
    }

    for (const auto& obs : rf_observations) {
        SpatialGraphEdge edge;
        edge.from_node = obs.reader_id;
        edge.to_node = obs.tag_id;
        edge.edge_type = SpatialEdgeType::READER_TAG;
        edge.source = EdgeSource::RF_ENVIRONMENT;
        edge.weight = std::max(0.0, (obs.rssi + 100.0) / 100.0);
        graph.edges.push_back(edge);
    }

    removeDanglingReferences(graph);

    graph.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();

    return graph;
}

} // namespace pa::spatial_truth