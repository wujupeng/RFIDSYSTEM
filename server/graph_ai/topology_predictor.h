#pragma once

#include "topology_snapshot.h"
#include <vector>

class TopologyPredictor {
public:
    TopologySnapshot predict(const std::vector<TopologySnapshot>& history,
                            int future_seconds);
    
    float predictNodeLoad(uint64_t node_id, 
                          const std::vector<TopologySnapshot>& history);
    
    float predictEdgeTraffic(uint64_t from_id, uint64_t to_id,
                            const std::vector<TopologySnapshot>& history);
    
    float predictFailureProbability(uint64_t node_id,
                                   const std::vector<TopologySnapshot>& history);

private:
    float calculateVelocity(const std::vector<float>& values);
    float calculateEMA(const std::vector<float>& values);
};