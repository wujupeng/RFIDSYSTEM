#include "factory_map_generator.h"
#include <chrono>
#include <algorithm>

namespace pa::spatial_truth {

FactoryMap FactoryMapGenerator::generate(const FactoryMapInput& input) const {
    FactoryMap map;

    map.spatial_graph = graph_builder_.build(
        input.readers, input.bindings, input.tag_estimates,
        input.zones, input.rf_observations
    );

    map.heatmap = heatmap_builder_.build(
        input.rf_env_data, input.heatmap_resolution,
        input.min_x, input.max_x, input.min_y, input.max_y
    );

    map.aoa_vectors = aoa_builder_.build(input.aoa_observations);

    map.node_status = status_encoder_.encodeAll(input.estimates);

    map.update_timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();

    return map;
}

FactoryMap FactoryMapGenerator::generateByZone(const FactoryMapInput& input, const std::string& zone_id) const {
    auto full = generate(input);
    FactoryMapPartitioner partitioner;
    auto parts = partitioner.partitionByZone(full);
    auto it = parts.find(zone_id);
    if (it != parts.end()) return it->second;
    return full;
}

std::map<std::string, FactoryMap> FactoryMapPartitioner::partitionByZone(const FactoryMap& factory_map) const {
    std::map<std::string, FactoryMap> result;

    for (const auto& node : factory_map.spatial_graph.nodes) {
        if (node.node_type != SpatialNodeType::ZONE) continue;

        FactoryMap part;
        part.spatial_graph.timestamp = factory_map.spatial_graph.timestamp;
        part.spatial_graph.nodes.push_back(node);
        part.update_timestamp = factory_map.update_timestamp;
        result[node.node_id] = part;
    }

    return result;
}

FactoryMap FactoryMapPartitioner::getViewport(const FactoryMap& factory_map,
                                               double min_x, double max_x,
                                               double min_y, double max_y) const {
    FactoryMap result;
    result.update_timestamp = factory_map.update_timestamp;
    result.spatial_graph.timestamp = factory_map.spatial_graph.timestamp;

    for (const auto& node : factory_map.spatial_graph.nodes) {
        auto x_it = node.attributes.find("x");
        auto y_it = node.attributes.find("y");
        if (x_it != node.attributes.end() && y_it != node.attributes.end()) {
            double x = std::stod(x_it->second);
            double y = std::stod(y_it->second);
            if (x >= min_x && x <= max_x && y >= min_y && y <= max_y) {
                result.spatial_graph.nodes.push_back(node);
            }
        } else {
            result.spatial_graph.nodes.push_back(node);
        }
    }

    return result;
}

} // namespace pa::spatial_truth