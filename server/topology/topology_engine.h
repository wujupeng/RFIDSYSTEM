#pragma once

#include "topology_types.h"
#include "tag_flow_tracker.h"
#include "rf_signal_analyzer.h"
#include "topology_graph_builder.h"
#include <mutex>

class TopologyEngine {
public:
    static TopologyEngine& instance();
    
    void initialize();
    void shutdown();
    
    void ingestRFIDEvent(const RFIDEvent& event);
    
    TopologyGraph buildGraph();
    
    void updateIncremental();
    
    const TopologyGraph& getCurrentGraph() const;
    
private:
    TopologyEngine();
    
    void updateNodes();
    void updateEdges();
    void pruneWeakEdges(float threshold);
    void applyTemporalSmoothing();
    
    TagFlowTracker flow_tracker_;
    RFSignalAnalyzer signal_analyzer_;
    TopologyGraphBuilder graph_builder_;
    TopologyGraph current_graph_;
    
    mutable std::mutex mutex_;
};