#pragma once

#include "../spatial_positioning/spatial_types.h"
#include <vector>

struct TrajectoryPrediction {
    std::vector<TagPosition> future_positions;
    double confidence;
    double time_horizon;
};

class TrajectoryPredictor {
public:
    static TrajectoryPredictor& instance();
    
    TrajectoryPrediction predict(const std::vector<TagPosition>& history, int future_seconds);
    
    void learnPattern(const std::vector<TagPosition>& trajectory);
    
private:
    TrajectoryPredictor();
};