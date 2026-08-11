#pragma once

#include "spatial_estimate.h"
#include "error_radius_calculator.h"
#include "trusted_judge.h"
#include "source_hash_calculator.h"
#include "../calibration_session/calibration_session.h"

namespace pa::spatial_truth {

struct FusionResult {
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
    pa::LocalizationMode mode = pa::LocalizationMode::RSSI_TRIANGULATION;
};

struct SpatialProbability {
    double covariance_xx = 0.0;
    double covariance_xy = 0.0;
    double covariance_yy = 0.0;
};

class SpatialEstimateBuilder {
public:
    SpatialEstimate build(
        const FusionResult& fusion,
        const SpatialProbability& prob,
        double confidence,
        uint64_t calibration_version,
        uint64_t source_hash,
        uint64_t environment_hash,
        double p95_from_calibration = -1.0
    );

private:
    ErrorRadiusCalculator error_radius_calc_;
    TrustedJudge trusted_judge_;

    void fixPositiveDefinite(SpatialProbability& prob) const;
};

} // namespace pa::spatial_truth