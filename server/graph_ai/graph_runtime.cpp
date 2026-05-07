#include "graph_runtime.h"

GraphRuntime::GraphRuntime()
    : topology_engine_(&TopologyEngine::instance()),
      self_healing_(&SelfHealingNetwork::instance()) {
}

GraphRuntime& GraphRuntime::instance() {
    static GraphRuntime instance;
    return instance;
}

void GraphRuntime::initialize() {
}

void GraphRuntime::shutdown() {
    history_.clear();
}

void GraphRuntime::tick() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto graph = topology_engine_->getCurrentGraph();
    
    TopologySnapshot snapshot;
    snapshot.timestamp = graph.timestamp;
    snapshot.nodes = graph.nodes;
    snapshot.edges = graph.edges;
    
    history_.push_back(snapshot);
    
    if (history_.size() > 100) {
        history_.erase(history_.begin());
    }
}

TopologySnapshot GraphRuntime::predict(int future_seconds) {
    std::lock_guard<std::mutex> lock(mutex_);
    return predictor_.predict(history_, future_seconds);
}

PropagationResult GraphRuntime::simulateFailure(uint64_t node_id) {
    auto graph = topology_engine_->getCurrentGraph();
    return propagation_engine_.simulateFailure(node_id, graph);
}

std::vector<OptimizationAction> GraphRuntime::optimize() {
    auto graph = topology_engine_->getCurrentGraph();
    return optimizer_.optimize(graph);
}

void GraphRuntime::onNodeFailure(uint64_t node_id) {
    self_healing_->onFailure(node_id);
}