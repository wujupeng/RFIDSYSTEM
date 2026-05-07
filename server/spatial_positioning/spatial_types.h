#pragma once

#include <cstdint>
#include <string>

struct TagObservation {
    std::string epc;
    double rssi;
    double phase;
    int antenna_id;
    uint64_t reader_id;
    double timestamp;
};

struct TagPosition {
    double x;
    double y;
    double confidence;
    double velocity_x;
    double velocity_y;
    uint64_t timestamp;
};

struct SpatialProbability {
    float mean_x;
    float mean_y;
    float covariance_xx;
    float covariance_xy;
    float covariance_yy;
    
    SpatialProbability() : mean_x(0), mean_y(0), 
                           covariance_xx(1), covariance_xy(0), covariance_yy(1) {}
};

struct ReaderInfo {
    uint64_t reader_id;
    double x;
    double y;
    double coverage_radius;
    int antenna_count;
};