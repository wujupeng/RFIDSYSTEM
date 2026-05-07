#pragma once

#include "spatial_types.h"
#include <vector>

class SpatialFilter {
public:
    SpatialFilter();
    
    TagPosition applyMovingAverage(const std::vector<TagPosition>& positions);
    
    TagPosition applyMedianFilter(const std::vector<TagPosition>& positions);
    
    TagPosition applyGaussianFilter(const std::vector<TagPosition>& positions);
    
    bool isValidPosition(const TagPosition& pos, 
                         const TagPosition& last_pos,
                         double max_speed = 10.0);
    
private:
    double gaussian(double x, double sigma = 1.0);
};