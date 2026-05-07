#pragma once

#include "../spatial_positioning/spatial_types.h"
#include <vector>

enum class AnomalyType {
    STATIONARY,
    SPEED,
    ZONE_VIOLATION,
    PATTERN_BREAK,
    CONFIDENCE_DROP
};

struct Anomaly {
    AnomalyType type;
    std::string epc;
    double confidence;
    std::string description;
    uint64_t timestamp;
};

class AnomalyDetector {
public:
    static AnomalyDetector& instance();
    
    std::vector<Anomaly> detect(const std::vector<TagPosition>& history,
                                const std::vector<TagPosition>& current);
    
    void setZoneBoundary(double min_x, double max_x, double min_y, double max_y);
    
private:
    AnomalyDetector();
    
    double min_x_, max_x_, min_y_, max_y_;
};