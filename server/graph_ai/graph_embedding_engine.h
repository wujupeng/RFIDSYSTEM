#pragma once

#include "../topology/topology_types.h"
#include <array>

struct NodeEmbedding {
    uint64_t node_id;
    std::array<float, 64> embedding;
    
    NodeEmbedding() : node_id(0) { embedding.fill(0.0f); }
};

class GraphEmbeddingEngine {
public:
    std::vector<NodeEmbedding> embed(const TopologyGraph& graph);
    
    float similarity(const NodeEmbedding& a, const NodeEmbedding& b) const;
    
    std::vector<uint64_t> findSimilarNodes(uint64_t node_id, const TopologyGraph& graph, int k);

private:
    void computeNodeFeatures(uint64_t node_id, const TopologyGraph& graph, float* features);
};