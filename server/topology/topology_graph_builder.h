#pragma once

#include "topology_types.h"
#include "tag_flow_tracker.h"
#include "rf_signal_analyzer.h"

class TopologyGraphBuilder {
public:
    TopologyGraphBuilder(TagFlowTracker& flow_tracker, RFSignalAnalyzer& signal_analyzer);
    
    TopologyGraph buildGraph();
    
    void addNode(uint64_t reader_id, float x, float y, float coverage_radius);
    
    void removeNode(uint64_t reader_id);
    
private:
    void buildEdges(TopologyGraph& graph);
    
    float calculateEdgeWeight(float overlap, float transition_prob, float correlation);
    
    TagFlowTracker& flow_tracker_;
    RFSignalAnalyzer& signal_analyzer_;
    
    std::vector<TopologyNode> nodes_;
};