#include "triangulation_engine.h"
#include <cmath>

TriangulationEngine::TriangulationEngine() {}

double TriangulationEngine::distance(double x1, double y1, double x2, double y2) {
    double dx = x2 - x1;
    double dy = y2 - y1;
    return sqrt(dx * dx + dy * dy);
}

TagPosition TriangulationEngine::triangulate(const std::vector<AngleMeasurement>& measurements) {
    return fromAngles(measurements);
}

TagPosition TriangulationEngine::fromDistances(const std::vector<ReaderInfo>& readers,
                                              const std::vector<double>& distances) {
    TagPosition pos;
    pos.confidence = 0.0;
    
    if (readers.size() < 2 || distances.size() != readers.size()) {
        return pos;
    }
    
    double sum_x = 0.0, sum_y = 0.0;
    double sum_weight = 0.0;
    
    for (size_t i = 0; i < readers.size(); ++i) {
        double d = distances[i];
        if (d < 0.01) d = 0.01;
        double weight = 1.0 / (d * d);
        
        sum_x += readers[i].x * weight;
        sum_y += readers[i].y * weight;
        sum_weight += weight;
    }
    
    if (sum_weight > 0) {
        pos.x = sum_x / sum_weight;
        pos.y = sum_y / sum_weight;
        pos.confidence = 0.7;
    }
    
    return pos;
}

TagPosition TriangulationEngine::fromAngles(const std::vector<AngleMeasurement>& measurements) {
    TagPosition pos;
    pos.confidence = 0.0;
    
    if (measurements.size() < 2) {
        return pos;
    }
    
    double sum_x = 0.0, sum_y = 0.0;
    double sum_weight = 0.0;
    
    for (const auto& m : measurements) {
        double weight = m.confidence;
        double extend_dist = 10.0;
        
        double end_x = m.reader_x + cos(m.angle_rad) * extend_dist;
        double end_y = m.reader_y + sin(m.angle_rad) * extend_dist;
        
        sum_x += end_x * weight;
        sum_y += end_y * weight;
        sum_weight += weight;
    }
    
    if (sum_weight > 0) {
        pos.x = sum_x / sum_weight;
        pos.y = sum_y / sum_weight;
        pos.confidence = sum_weight / measurements.size();
    }
    
    return pos;
}