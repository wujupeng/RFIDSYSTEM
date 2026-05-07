#include "topology_graph_builder.h"

TopologyGraphBuilder::TopologyGraphBuilder(TagFlowTracker& flow_tracker, RFSignalAnalyzer& signal_analyzer)
    : flow_tracker_(flow_tracker), signal_analyzer_(signal_analyzer) {
}

TopologyGraph TopologyGraphBuilder::buildGraph() {
    TopologyGraph graph;
    graph.nodes = nodes_;
    
    buildEdges(graph);
    
    graph.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    
    return graph;
}

void TopologyGraphBuilder::addNode(uint64_t reader_id, float x, float y, float coverage_radius) {
    TopologyNode node;
    node.reader_id = reader_id;
    node.x = x;
    node.y = y;
    node.coverage_radius = coverage_radius;
    node.load = 0.0f;
    node.online = true;
    
    auto it = std::find_if(nodes_.begin(), nodes_.end(), 
        [reader_id](const TopologyNode& n) { return n.reader_id == reader_id; });
    
    if (it != nodes_.end()) {
        *it = node;
    } else {
        nodes_.push_back(node);
    }
}

void TopologyGraphBuilder::removeNode(uint64_t reader_id) {
    auto it = std::remove_if(nodes_.begin(), nodes_.end(),
        [reader_id](const TopologyNode& n) { return n.reader_id == reader_id; });
    nodes_.erase(it, nodes_.end());
}

void TopologyGraphBuilder::buildEdges(TopologyGraph& graph) {
    for (size_t i = 0; i < nodes_.size(); ++i) {
        for (size_t j = i + 1; j < nodes_.size(); ++j) {
            uint64_t id_a = nodes_[i].reader_id;
            uint64_t id_b = nodes_[j].reader_id;
            
            float overlap = signal_analyzer_.calculateOverlap(id_a, id_b);
            
            float trans_ab = flow_tracker_.getTransitionProbability(id_a, id_b);
            float trans_ba = flow_tracker_.getTransitionProbability(id_b, id_a);
            float transition_prob = (trans_ab + trans_ba) / 2.0f;
            
            float correlation = 0.5f;
            
            float weight_ab = calculateEdgeWeight(overlap, trans_ab, correlation);
            float weight_ba = calculateEdgeWeight(overlap, trans_ba, correlation);
            
            if (weight_ab > 0.1f) {
                TopologyEdge edge_ab;
                edge_ab.from = id_a;
                edge_ab.to = id_b;
                edge_ab.strength = weight_ab;
                edge_ab.transition_prob = trans_ab;
                edge_ab.overlap = overlap;
                edge_ab.correlation = correlation;
                graph.edges.push_back(edge_ab);
            }
            
            if (weight_ba > 0.1f && weight_ba != weight_ab) {
                TopologyEdge edge_ba;
                edge_ba.from = id_b;
                edge_ba.to = id_a;
                edge_ba.strength = weight_ba;
                edge_ba.transition_prob = trans_ba;
                edge_ba.overlap = overlap;
                edge_ba.correlation = correlation;
                graph.edges.push_back(edge_ba);
            }
        }
    }
}

float TopologyGraphBuilder::calculateEdgeWeight(float overlap, float transition_prob, float correlation) {
    return 0.4f * overlap + 0.4f * transition_prob + 0.2f * correlation;
}