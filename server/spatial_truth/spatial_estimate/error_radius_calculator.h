#pragma once

#include "spatial_estimate.h"
#include <vector>

namespace pa::spatial_truth {

class ErrorRadiusCalculator {
public:
    double calculate(double cov_xx, double cov_xy, double cov_yy,
                     double confidence_level = 0.95) const;
    double calculateFromP95(double p95) const;
};

} // namespace pa::spatial_truth