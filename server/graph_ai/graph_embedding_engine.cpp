#include "graph_embedding_engine.h"
#include <cmath>
#include <algorithm>

std::vector<NodeEmbedding> GraphEmbeddingEngine::embed(const TopologyGraph& graph) {
    std::vector<NodeEmbedding> embeddings;
    
    for (const auto& node : graph.nodes) {
        NodeEmbedding embedding;
        embedding.node_id = node.reader_id;
        
        computeNodeFeatures(node.reader_id, graph, embedding.embedding.data());
        
        embeddings.push_back(embedding);
    }
    
    return embeddings;
}

float GraphEmbeddingEngine::similarity(const NodeEmbedding& a, const NodeEmbedding& b) const {
    float dot = 0.0f;
    float norm_a = 0.0f;
    float norm_b = 0.0f;
    
    for (size_t i = 0; i < 64; ++i) {
        dot += a.embedding[i] * b.embedding[i];
        norm_a += a.embedding[i] * a.embedding[i];
        norm_b += b.embedding[i] * b.embedding[i];
    }
    
    if (norm_a == 0 || norm_b == 0) return 0.0f;
    
    return dot / (std::sqrt(norm_a) * std::sqrt(norm_b));
}

std::vector<uint64_t> GraphEmbeddingEngine::findSimilarNodes(uint64_t node_id, 
                                                            const TopologyGraph& graph, 
                                                            int k) {
    auto embeddings = embed(graph);
    
    auto it = std::find_if(embeddings.begin(), embeddings.end(),
        [node_id](const NodeEmbedding& e) { return e.node_id == node_id; });
    
    if (it == embeddings.end()) return {};
    
    const NodeEmbedding& target = *it;
    
    std::vector<std::pair<float, uint64_t>> similarities;
    for (const auto& e : embeddings) {
        if (e.node_id != node_id) {
            similarities.emplace_back(similarity(target, e), e.node_id);
        }
    }
    
    std::sort(similarities.rbegin(), similarities.rend());
    
    std::vector<uint64_t> result;
    for (int i = 0; i < k && i < similarities.size(); ++i) {
        result.push_back(similarities[i].second);
    }
    
    return result;
}

void GraphEmbeddingEngine::computeNodeFeatures(uint64_t node_id, 
                                              const TopologyGraph& graph, 
                                              float* features) {
    auto it = std::find_if(graph.nodes.begin(), graph.nodes.end(),
        [node_id](const TopologyNode& n) { return n.reader_id == node_id; });
    
    if (it == graph.nodes.end()) return;
    
    const TopologyNode& node = *it;
    
    features[0] = node.x;
    features[1] = node.y;
    features[2] = node.coverage_radius;
    features[3] = node.load;
    features[4] = node.online ? 1.0f : 0.0f;
    
    float avg_edge_strength = 0.0f;
    int edge_count = 0;
    for (const auto& edge : graph.edges) {
        if (edge.from == node_id || edge.to == node_id) {
            avg_edge_strength += edge.strength;
            edge_count++;
        }
    }
    
    if (edge_count > 0) {
        avg_edge_strength /= edge_count;
    }
    
    features[5] = avg_edge_strength;
    features[6] = static_cast<float>(edge_count);
    
    for (size_t i = 7; i < 64; ++i) {
        features[i] = 0.0f;
    }
}