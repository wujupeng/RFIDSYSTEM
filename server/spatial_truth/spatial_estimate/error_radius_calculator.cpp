#include "error_radius_calculator.h"
#include <cmath>
#include <algorithm>

namespace pa::spatial_truth {

double ErrorRadiusCalculator::calculate(double cov_xx, double cov_xy, double cov_yy,
                                         double confidence_level) const {
    double trace = cov_xx + cov_yy;
    double det = cov_xx * cov_yy - cov_xy * cov_xy;
    double discriminant = std::max(0.0, trace * trace - 4.0 * det);
    double lambda_max = (trace + std::sqrt(discriminant)) / 2.0;
    if (lambda_max < 0.0) lambda_max = 0.0;

    double z_score = 1.96;
    if (confidence_level < 0.99) z_score = 1.96;
    else z_score = 2.576;

    return z_score * std::sqrt(lambda_max);
}

double ErrorRadiusCalculator::calculateFromP95(double p95) const {
    return p95;
}

} // namespace pa::spatial_truth