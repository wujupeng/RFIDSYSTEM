#pragma once

#include "../topology/topology_types.h"
#include <vector>

struct PropagationResult {
    uint64_t source_node;
    
    std::vector<uint64_t> impacted_nodes;
    std::vector<double> node_risks;
    
    double total_risk;
};

class FailurePropagationEngine {
public:
    PropagationResult simulateFailure(uint64_t node_id, const TopologyGraph& graph);
    
    std::vector<uint64_t> findCriticalPaths(uint64_t source_id, const TopologyGraph& graph);
    
    double calculateRiskPropagation(uint64_t source_id, uint64_t target_id, 
                                   const TopologyGraph& graph);

private:
    void propagateRisk(uint64_t current_id, uint64_t source_id, double risk,
                       const TopologyGraph& graph, std::vector<double>& risks,
                       std::vector<bool>& visited, double distance);
};