#include "trusted_judge.h"

namespace pa::spatial_truth {

bool TrustedJudge::judge(double confidence, double error_radius, uint64_t calibration_version) const {
    return confidence >= confidence_threshold_ &&
           error_radius <= error_radius_threshold_ &&
           calibration_version > 0;
}

} // namespace pa::spatial_truth