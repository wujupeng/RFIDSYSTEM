#pragma once

#include "spatial_types.h"
#include <vector>

class PhaseLocalizer {
public:
    PhaseLocalizer();
    
    double calculatePhaseDifference(double phase1, double phase2);
    
    double estimateAngle(double phase_diff, double antenna_spacing_m = 0.05, 
                         double wavelength_m = 0.122);
    
    SpatialProbability localize(const std::vector<TagObservation>& observations,
                                const std::vector<ReaderInfo>& readers);
    
private:
    double wrapToPi(double angle);
};