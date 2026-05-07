#include "phase_localizer.h"
#include <cmath>

PhaseLocalizer::PhaseLocalizer() {}

double PhaseLocalizer::wrapToPi(double angle) {
    while (angle > M_PI) angle -= 2 * M_PI;
    while (angle < -M_PI) angle += 2 * M_PI;
    return angle;
}

double PhaseLocalizer::calculatePhaseDifference(double phase1, double phase2) {
    double diff = phase2 - phase1;
    return wrapToPi(diff);
}

double PhaseLocalizer::estimateAngle(double phase_diff, double antenna_spacing_m, 
                                    double wavelength_m) {
    double sin_theta = (phase_diff * wavelength_m) / (2 * M_PI * antenna_spacing_m);
    sin_theta = std::max(-1.0, std::min(1.0, sin_theta));
    return asin(sin_theta);
}

SpatialProbability PhaseLocalizer::localize(const std::vector<TagObservation>& observations,
                                           const std::vector<ReaderInfo>& readers) {
    SpatialProbability prob;
    
    if (observations.size() < 2) {
        return prob;
    }
    
    auto it = std::find_if(readers.begin(), readers.end(),
        [&](const ReaderInfo& r) { return r.reader_id == observations[0].reader_id; });
    
    if (it != readers.end()) {
        prob.mean_x = it->x;
        prob.mean_y = it->y;
        
        double phase_sum = 0.0;
        for (size_t i = 1; i < observations.size(); ++i) {
            double diff = calculatePhaseDifference(observations[0].phase, observations[i].phase);
            phase_sum += diff;
        }
        
        double avg_phase_diff = phase_sum / (observations.size() - 1);
        double angle = estimateAngle(avg_phase_diff);
        
        prob.covariance_xx = 4.0;
        prob.covariance_yy = 4.0;
    }
    
    return prob;
}