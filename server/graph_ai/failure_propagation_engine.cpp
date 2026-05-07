#include "failure_propagation_engine.h"
#include <cmath>
#include <algorithm>

PropagationResult FailurePropagationEngine::simulateFailure(uint64_t node_id, const TopologyGraph& graph) {
    PropagationResult result;
    result.source_node = node_id;
    result.total_risk = 0.0;
    
    std::vector<double> risks(graph.nodes.size(), 0.0);
    std::vector<bool> visited(graph.nodes.size(), false);
    
    auto it = std::find_if(graph.nodes.begin(), graph.nodes.end(),
        [node_id](const TopologyNode& n) { return n.reader_id == node_id; });
    
    if (it == graph.nodes.end()) {
        return result;
    }
    
    int source_idx = std::distance(graph.nodes.begin(), it);
    risks[source_idx] = 1.0;
    
    propagateRisk(node_id, node_id, 1.0, graph, risks, visited, 0.0);
    
    for (size_t i = 0; i < graph.nodes.size(); ++i) {
        if (risks[i] > 0.01 && graph.nodes[i].reader_id != node_id) {
            result.impacted_nodes.push_back(graph.nodes[i].reader_id);
            result.node_risks.push_back(risks[i]);
            result.total_risk += risks[i];
        }
    }
    
    return result;
}

std::vector<uint64_t> FailurePropagationEngine::findCriticalPaths(uint64_t source_id, const TopologyGraph& graph) {
    std::vector<uint64_t> critical_nodes;
    
    auto result = simulateFailure(source_id, graph);
    
    for (size_t i = 0; i < result.impacted_nodes.size(); ++i) {
        if (result.node_risks[i] > 0.3) {
            critical_nodes.push_back(result.impacted_nodes[i]);
        }
    }
    
    return critical_nodes;
}

double FailurePropagationEngine::calculateRiskPropagation(uint64_t source_id, uint64_t target_id,
                                                         const TopologyGraph& graph) {
    auto result = simulateFailure(source_id, graph);
    
    auto it = std::find(result.impacted_nodes.begin(), result.impacted_nodes.end(), target_id);
    if (it != result.impacted_nodes.end()) {
        size_t idx = std::distance(result.impacted_nodes.begin(), it);
        return result.node_risks[idx];
    }
    
    return 0.0;
}

void FailurePropagationEngine::propagateRisk(uint64_t current_id, uint64_t source_id, double risk,
                                            const TopologyGraph& graph, std::vector<double>& risks,
                                            std::vector<bool>& visited, double distance) {
    auto current_it = std::find_if(graph.nodes.begin(), graph.nodes.end(),
        [current_id](const TopologyNode& n) { return n.reader_id == current_id; });
    
    if (current_it == graph.nodes.end()) return;
    
    int current_idx = std::distance(graph.nodes.begin(), current_it);
    if (visited[current_idx]) return;
    
    visited[current_idx] = true;
    
    for (const auto& edge : graph.edges) {
        if (edge.from == current_id) {
            auto target_it = std::find_if(graph.nodes.begin(), graph.nodes.end(),
                [edge.to](const TopologyNode& n) { return n.reader_id == edge.to; });
            
            if (target_it != graph.nodes.end()) {
                int target_idx = std::distance(graph.nodes.begin(), target_it);
                double propagated_risk = risk * edge.strength * std::exp(-distance * 0.1);
                
                if (propagated_risk > risks[target_idx]) {
                    risks[target_idx] = propagated_risk;
                    propagateRisk(edge.to, source_id, propagated_risk, 
                                graph, risks, visited, distance + 1.0);
                }
            }
        }
    }
}