#include "aoa_solver.h"
#include <cmath>
#include <algorithm>

AoASolver::AoASolver() {}

double AoASolver::wrapToPi(double angle) {
    while (angle > M_PI) angle -= 2 * M_PI;
    while (angle < -M_PI) angle += 2 * M_PI;
    return angle;
}

double AoASolver::estimateAngleMUSIC(const std::vector<double>& phases,
                                    double antenna_spacing_m,
                                    double wavelength_m) {
    if (phases.size() < 2) {
        return 0.0;
    }
    
    double sum_phase_diff = 0.0;
    for (size_t i = 1; i < phases.size(); ++i) {
        sum_phase_diff += wrapToPi(phases[i] - phases[0]);
    }
    
    double avg_diff = sum_phase_diff / (phases.size() - 1);
    double sin_theta = (avg_diff * wavelength_m) / (2 * M_PI * antenna_spacing_m);
    sin_theta = std::max(-1.0, std::min(1.0, sin_theta));
    
    return asin(sin_theta);
}

double AoASolver::estimateAngleESPRIT(const std::vector<double>& phases,
                                     double antenna_spacing_m,
                                     double wavelength_m) {
    if (phases.size() < 2) {
        return 0.0;
    }
    
    double sum_diff = 0.0;
    int count = 0;
    
    for (size_t i = 0; i < phases.size() - 1; ++i) {
        double diff = wrapToPi(phases[i + 1] - phases[i]);
        sum_diff += diff;
        count++;
    }
    
    if (count == 0) {
        return 0.0;
    }
    
    double avg_diff = sum_diff / count;
    double sin_theta = (avg_diff * wavelength_m) / (2 * M_PI * antenna_spacing_m);
    sin_theta = std::max(-1.0, std::min(1.0, sin_theta));
    
    return asin(sin_theta);
}

double AoASolver::solve(const std::vector<TagObservation>& observations) {
    std::vector<double> phases;
    for (const auto& obs : observations) {
        phases.push_back(obs.phase);
    }
    
    if (phases.empty()) {
        return 0.0;
    }
    
    return estimateAngleESPRIT(phases);
}