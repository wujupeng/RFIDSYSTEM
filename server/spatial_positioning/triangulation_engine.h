#pragma once

#include "spatial_types.h"
#include <vector>

struct AngleMeasurement {
    double reader_x;
    double reader_y;
    double angle_rad;
    double confidence;
};

class TriangulationEngine {
public:
    TriangulationEngine();
    
    TagPosition triangulate(const std::vector<AngleMeasurement>& measurements);
    
    TagPosition fromDistances(const std::vector<ReaderInfo>& readers,
                             const std::vector<double>& distances);
    
    TagPosition fromAngles(const std::vector<AngleMeasurement>& measurements);
    
private:
    double distance(double x1, double y1, double x2, double y2);
};