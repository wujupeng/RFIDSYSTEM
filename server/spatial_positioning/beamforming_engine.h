#pragma once

#include "spatial_types.h"
#include <vector>

struct Beam {
    double angle;
    double power;
    double confidence;
};

struct SpatialEnergyMap {
    double resolution;
    std::vector<std::vector<double>> energy;
    int width;
    int height;
};

class BeamformingEngine {
public:
    BeamformingEngine();
    
    std::vector<Beam> formBeams(const std::vector<TagObservation>& observations);
    
    SpatialEnergyMap scanSpace(const std::vector<TagObservation>& observations,
                               double min_angle = -M_PI,
                               double max_angle = M_PI,
                               double resolution = 0.01);
    
    TagPosition findMaxEnergyPosition(const SpatialEnergyMap& map,
                                      double origin_x,
                                      double origin_y);
    
private:
    double calculateBeamPower(const std::vector<double>& phases, double target_angle);
};