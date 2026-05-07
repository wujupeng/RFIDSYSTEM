#pragma once

#include "spatial_types.h"
#include <vector>

class RSSILocalizer {
public:
    RSSILocalizer();
    
    double estimateDistance(double rssi, double tx_power = 30.0, double n = 2.0);
    
    SpatialProbability localize(const std::vector<TagObservation>& observations,
                                const std::vector<ReaderInfo>& readers);
    
private:
    double calculateDistance(double rssi);
};