#include "graph_optimizer.h"
#include <algorithm>
#include <cmath>

std::vector<OptimizationAction> GraphOptimizer::optimize(const TopologyGraph& graph) {
    std::vector<OptimizationAction> actions;
    
    auto load_actions = optimizeLoadBalance(graph);
    auto coverage_actions = optimizeCoverage(graph);
    auto congestion_actions = reduceCongestion(graph);
    
    actions.insert(actions.end(), load_actions.begin(), load_actions.end());
    actions.insert(actions.end(), coverage_actions.begin(), coverage_actions.end());
    actions.insert(actions.end(), congestion_actions.begin(), congestion_actions.end());
    
    return actions;
}

std::vector<OptimizationAction> GraphOptimizer::optimizeLoadBalance(const TopologyGraph& graph) {
    std::vector<OptimizationAction> actions;
    
    if (graph.nodes.empty()) return actions;
    
    float total_load = 0.0f;
    for (const auto& node : graph.nodes) {
        total_load += node.load;
    }
    
    float avg_load = total_load / graph.nodes.size();
    
    for (const auto& node : graph.nodes) {
        if (node.load > avg_load * 1.5f) {
            OptimizationAction action;
            action.type = OptimizationAction::REBALANCE_LOAD;
            action.target = node.reader_id;
            action.parameter = node.load - avg_load;
            actions.push_back(action);
        }
    }
    
    return actions;
}

std::vector<OptimizationAction> GraphOptimizer::optimizeCoverage(const TopologyGraph& graph) {
    std::vector<OptimizationAction> actions;
    
    for (const auto& node : graph.nodes) {
        if (node.load < 0.1f && node.online) {
            OptimizationAction action;
            action.type = OptimizationAction::CHANGE_POWER;
            action.target = node.reader_id;
            action.parameter = 1.5f;
            actions.push_back(action);
        }
    }
    
    return actions;
}

std::vector<OptimizationAction> GraphOptimizer::reduceCongestion(const TopologyGraph& graph) {
    std::vector<OptimizationAction> actions;
    
    for (const auto& edge : graph.edges) {
        if (edge.strength > 0.8f) {
            OptimizationAction action;
            action.type = OptimizationAction::REDIRECT_TRAFFIC;
            action.target = edge.from;
            action.parameter = edge.strength;
            actions.push_back(action);
        }
    }
    
    return actions;
}

double GraphOptimizer::calculateObjective(const TopologyGraph& graph) {
    double congestion = 0.0;
    double imbalance = 0.0;
    
    if (!graph.nodes.empty()) {
        float total_load = 0.0f;
        for (const auto& node : graph.nodes) {
            total_load += node.load;
            congestion += std::pow(node.load, 2);
        }
        
        float avg_load = total_load / graph.nodes.size();
        for (const auto& node : graph.nodes) {
            imbalance += std::abs(node.load - avg_load);
        }
    }
    
    double edge_congestion = 0.0;
    for (const auto& edge : graph.edges) {
        edge_congestion += std::pow(edge.strength, 2);
    }
    
    return congestion + imbalance + edge_congestion;
}