#include "tag_asset_binding_sync.h"

namespace pa::spatial_truth {

std::vector<SpatialGraphEdge> TagAssetBindingSync::sync(const std::vector<TagAssetBinding>& bindings) const {
    std::vector<SpatialGraphEdge> edges;
    for (const auto& b : bindings) {
        SpatialGraphEdge edge;
        edge.from_node = b.tag_id;
        edge.to_node = b.asset_id;
        edge.edge_type = SpatialEdgeType::TAG_ASSET;
        edge.source = EdgeSource::SPATIAL_TOPOLOGY;
        edge.weight = 1.0;
        edges.push_back(edge);
    }
    return edges;
}

std::vector<SpatialGraphNode> TagAssetBindingSync::buildNodes(const std::vector<TagAssetBinding>& bindings) const {
    std::vector<SpatialGraphNode> nodes;
    std::string last_tag;
    for (const auto& b : bindings) {
        if (b.tag_id != last_tag) {
            SpatialGraphNode tag_node;
            tag_node.node_id = b.tag_id;
            tag_node.node_type = SpatialNodeType::TAG;
            nodes.push_back(tag_node);
            last_tag = b.tag_id;
        }
        SpatialGraphNode asset_node;
        asset_node.node_id = b.asset_id;
        asset_node.node_type = SpatialNodeType::ASSET;
        nodes.push_back(asset_node);
    }
    return nodes;
}

std::vector<SpatialGraphEdge> SpatialOwnershipResolver::resolve(
    const std::vector<TagEstimate>& tag_estimates,
    const std::vector<ZoneInfo>& zones
) const {
    std::vector<SpatialGraphEdge> edges;
    for (const auto& tag : tag_estimates) {
        for (const auto& zone : zones) {
            if (tag.x >= zone.min_x && tag.x <= zone.max_x &&
                tag.y >= zone.min_y && tag.y <= zone.max_y) {
                SpatialGraphEdge edge;
                edge.from_node = tag.tag_id;
                edge.to_node = zone.zone_id;
                edge.edge_type = SpatialEdgeType::ASSET_ZONE;
                edge.source = EdgeSource::SPATIAL_TOPOLOGY;
                edge.weight = 1.0;
                edges.push_back(edge);
                break;
            }
        }
    }
    return edges;
}

} // namespace pa::spatial_truth