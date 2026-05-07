#pragma once

#include "../topology/topology_types.h"
#include "failure_propagation_engine.h"
#include "graph_optimizer.h"
#include "../kernel/proof_ledger.h"

class SelfHealingNetwork {
public:
    static SelfHealingNetwork& instance();
    
    void onFailure(uint64_t node_id);
    
    bool isHealing() const;
    
    int getHealingCount() const;
    
private:
    SelfHealingNetwork();
    
    void isolateNode(uint64_t id);
    
    void rerouteTraffic(uint64_t failed_id);
    
    void enableBackupReader(uint64_t failed_id);
    
    void rebalanceLoad();
    
    void verifyAndRecord(uint64_t failed_id);
    
    FailurePropagationEngine propagation_engine_;
    GraphOptimizer optimizer_;
    
    bool is_healing_ = false;
    int healing_count_ = 0;
};