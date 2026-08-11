#pragma once

#include "factory_map.h"
#include <vector>

namespace pa::spatial_truth {

struct RFEnvironmentData {
    double x = 0.0;
    double y = 0.0;
    double rssi = 0.0;
    double interference = 0.0;
};

class HeatmapBuilder {
public:
    Heatmap build(const std::vector<RFEnvironmentData>& rf_data, double resolution,
                  double min_x, double max_x, double min_y, double max_y) const;
};

struct AoAObservation {
    std::string reader_id;
    double reader_x = 0.0;
    double reader_y = 0.0;
    double angle = 0.0;
};

class AoAVectorBuilder {
public:
    std::vector<AoAVector> build(const std::vector<AoAObservation>& observations) const;
};

} // namespace pa::spatial_truth