#include "topology_engine.h"
#include <algorithm>

TopologyEngine::TopologyEngine()
    : graph_builder_(flow_tracker_, signal_analyzer_) {
}

TopologyEngine& TopologyEngine::instance() {
    static TopologyEngine engine;
    return engine;
}

void TopologyEngine::initialize() {
}

void TopologyEngine::shutdown() {
    flow_tracker_.clear();
}

void TopologyEngine::ingestRFIDEvent(const RFIDEvent& event) {
    flow_tracker_.trackEvent(event);
    signal_analyzer_.updateSignalStrength(event.reader_id, event.rssi);
    
    graph_builder_.addNode(event.reader_id, event.x, event.y, 3.0f);
}

TopologyGraph TopologyEngine::buildGraph() {
    std::lock_guard<std::mutex> lock(mutex_);
    return graph_builder_.buildGraph();
}

void TopologyEngine::updateIncremental() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    updateNodes();
    updateEdges();
    pruneWeakEdges(0.1f);
    applyTemporalSmoothing();
    
    current_graph_ = graph_builder_.buildGraph();
}

const TopologyGraph& TopologyEngine::getCurrentGraph() const {
    return current_graph_;
}

void TopologyEngine::updateNodes() {
}

void TopologyEngine::updateEdges() {
}

void TopologyEngine::pruneWeakEdges(float threshold) {
    auto& edges = current_graph_.edges;
    auto it = std::remove_if(edges.begin(), edges.end(),
        [threshold](const TopologyEdge& e) { return e.strength < threshold; });
    edges.erase(it, edges.end());
}

void TopologyEngine::applyTemporalSmoothing() {
    for (auto& edge : current_graph_.edges) {
        edge.strength = 0.8f * edge.strength + 0.2f * (edge.overlap + edge.transition_prob) / 2.0f;
    }
}