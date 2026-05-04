#include "acquisition_function.h"
#include <cmath>
#include <algorithm>

AcquisitionFunction::AcquisitionFunction()
    : type_(AcquisitionType::UCB), beta_(2.0), epsilon_(0.01) {
}

double AcquisitionFunction::compute(double mean, double uncertainty, double beta) {
    switch (type_) {
        case AcquisitionType::UCB:
            return upperConfidenceBound(mean, uncertainty, beta);
        case AcquisitionType::EI:
            return expectedImprovement(mean, uncertainty, 0.0, epsilon_);
        case AcquisitionType::PI:
            return probabilityOfImprovement(mean, uncertainty, 0.0, epsilon_);
        default:
            return upperConfidenceBound(mean, uncertainty, beta);
    }
}

double AcquisitionFunction::upperConfidenceBound(double mean,
                                                 double uncertainty,
                                                 double beta) {
    return mean + beta * uncertainty;
}

double AcquisitionFunction::expectedImprovement(double mean,
                                              double uncertainty,
                                              double best_reward,
                                              double epsilon) {
    if (uncertainty < 1e-6) {
        return 0.0;
    }

    double z = (mean - best_reward - epsilon) / uncertainty;
    double normal_cdf = 0.5 * (1.0 + std::erf(z / std::sqrt(2.0)));
    double normal_pdf = std::exp(-0.5 * z * z) / std::sqrt(2.0 * M_PI);

    double ei = (mean - best_reward - epsilon) * normal_cdf + uncertainty * normal_pdf;

    return std::max(0.0, ei);
}

double AcquisitionFunction::probabilityOfImprovement(double mean,
                                                     double uncertainty,
                                                     double best_reward,
                                                     double epsilon) {
    if (uncertainty < 1e-6) {
        return (mean > best_reward) ? 1.0 : 0.0;
    }

    double z = (mean - best_reward - epsilon) / uncertainty;
    return 0.5 * (1.0 + std::erf(z / std::sqrt(2.0)));
}
