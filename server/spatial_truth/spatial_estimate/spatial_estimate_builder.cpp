#include "spatial_estimate_builder.h"
#include <cmath>

namespace pa::spatial_truth {

void SpatialEstimateBuilder::fixPositiveDefinite(SpatialProbability& prob) const {
    double det = prob.covariance_xx * prob.covariance_yy - prob.covariance_xy * prob.covariance_xy;
    if (det <= 0.0) {
        prob.covariance_xy = 0.0;
        prob.covariance_xx = std::abs(prob.covariance_xx);
        prob.covariance_yy = std::abs(prob.covariance_yy);
        const double epsilon = 1e-9;
        if (prob.covariance_xx < epsilon) prob.covariance_xx = epsilon;
        if (prob.covariance_yy < epsilon) prob.covariance_yy = epsilon;
    }
}

SpatialEstimate SpatialEstimateBuilder::build(
    const FusionResult& fusion,
    const SpatialProbability& prob,
    double confidence,
    uint64_t calibration_version,
    uint64_t source_hash,
    uint64_t environment_hash,
    double p95_from_calibration
) {
    SpatialEstimate est;
    est.x = fusion.x;
    est.y = fusion.y;
    est.z = fusion.z;
    est.mode = fusion.mode;
    est.confidence = confidence;
    est.calibration_version = calibration_version;
    est.source_hash = source_hash;
    est.environment_hash = environment_hash;

    SpatialProbability fixed_prob = prob;
    fixPositiveDefinite(fixed_prob);
    est.covariance_xx = fixed_prob.covariance_xx;
    est.covariance_xy = fixed_prob.covariance_xy;
    est.covariance_yy = fixed_prob.covariance_yy;

    if (p95_from_calibration > 0.0) {
        est.error_radius = error_radius_calc_.calculateFromP95(p95_from_calibration);
    } else {
        est.error_radius = error_radius_calc_.calculate(
            est.covariance_xx, est.covariance_xy, est.covariance_yy
        );
    }

    double orig_det = prob.covariance_xx * prob.covariance_yy - prob.covariance_xy * prob.covariance_xy;
    if (orig_det <= 0.0) {
        est.error_radius = 1.0;
    }

    est.trusted = trusted_judge_.judge(est.confidence, est.error_radius, est.calibration_version);

    return est;
}

} // namespace pa::spatial_truth