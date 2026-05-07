#pragma once

#include "topology_predictor.h"
#include "failure_propagation_engine.h"
#include "graph_optimizer.h"
#include "self_healing_network.h"
#include "graph_embedding_engine.h"
#include "../topology/topology_engine.h"
#include <mutex>

class GraphRuntime {
public:
    static GraphRuntime& instance();
    
    void initialize();
    void shutdown();
    
    void tick();
    
    TopologySnapshot predict(int future_seconds);
    
    PropagationResult simulateFailure(uint64_t node_id);
    
    std::vector<OptimizationAction> optimize();
    
    void onNodeFailure(uint64_t node_id);
    
private:
    GraphRuntime();
    
    TopologyEngine* topology_engine_;
    TopologyPredictor predictor_;
    FailurePropagationEngine propagation_engine_;
    GraphOptimizer optimizer_;
    SelfHealingNetwork* self_healing_;
    GraphEmbeddingEngine embedding_engine_;
    
    std::vector<TopologySnapshot> history_;
    mutable std::mutex mutex_;
};