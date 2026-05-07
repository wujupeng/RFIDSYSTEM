#pragma once

#include "spatial_types.h"
#include <vector>

class AoASolver {
public:
    AoASolver();
    
    double estimateAngleMUSIC(const std::vector<double>& phases, 
                             double antenna_spacing_m = 0.05,
                             double wavelength_m = 0.122);
    
    double estimateAngleESPRIT(const std::vector<double>& phases,
                              double antenna_spacing_m = 0.05,
                              double wavelength_m = 0.122);
    
    double solve(const std::vector<TagObservation>& observations);
    
private:
    double wrapToPi(double angle);
};