#pragma once

#include "../topology/topology_types.h"
#include <vector>
#include <string>

struct OptimizationAction {
    enum Type {
        CHANGE_POWER,
        CHANGE_CHANNEL,
        REDIRECT_TRAFFIC,
        ENABLE_BACKUP_READER,
        REBALANCE_LOAD
    };
    
    Type type;
    uint64_t target;
    float parameter;
};

class GraphOptimizer {
public:
    std::vector<OptimizationAction> optimize(const TopologyGraph& graph);
    
    std::vector<OptimizationAction> optimizeLoadBalance(const TopologyGraph& graph);
    
    std::vector<OptimizationAction> optimizeCoverage(const TopologyGraph& graph);
    
    std::vector<OptimizationAction> reduceCongestion(const TopologyGraph& graph);
    
private:
    double calculateObjective(const TopologyGraph& graph);
};