#pragma once

#include "spatial_estimate.h"

namespace pa::spatial_truth {

class TrustedJudge {
public:
    bool judge(double confidence, double error_radius, uint64_t calibration_version) const;

    void setThresholds(double conf_threshold, double error_radius_threshold) {
        confidence_threshold_ = conf_threshold;
        error_radius_threshold_ = error_radius_threshold;
    }

private:
    double confidence_threshold_ = 0.7;
    double error_radius_threshold_ = 0.20;
};

} // namespace pa::spatial_truth