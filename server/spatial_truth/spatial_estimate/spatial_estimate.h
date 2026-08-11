#pragma once

#include "../calibration_session/calibration_session.h"
#include <string>
#include <cstdint>

namespace pa::spatial_truth {

struct SpatialEstimate {
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
    double confidence = 0.0;
    double covariance_xx = 0.0;
    double covariance_xy = 0.0;
    double covariance_yy = 0.0;
    double error_radius = 0.0;
    uint64_t source_hash = 0;
    uint64_t environment_hash = 0;
    uint64_t calibration_version = 0;
    pa::LocalizationMode mode = pa::LocalizationMode::RSSI_TRIANGULATION;
    bool trusted = false;
};

struct TagPosition {
    double x = 0.0;
    double y = 0.0;
    double confidence = 0.0;
    double velocity_x = 0.0;
    double velocity_y = 0.0;
    uint64_t timestamp = 0;
};

inline TagPosition toTagPosition(const SpatialEstimate& est, uint64_t timestamp = 0) {
    TagPosition tp;
    tp.x = est.x;
    tp.y = est.y;
    tp.confidence = est.confidence;
    tp.velocity_x = 0.0;
    tp.velocity_y = 0.0;
    tp.timestamp = timestamp;
    return tp;
}

} // namespace pa::spatial_truth