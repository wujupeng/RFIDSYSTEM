#pragma once

#include "calibration_session.h"
#include <vector>
#include <string>

namespace pa::spatial_truth {

struct AccuracyReport {
    double position_error = 0.0;
    double x_error = 0.0;
    double y_error = 0.0;
    double rmse = 0.0;
    double mae = 0.0;
    double p50 = 0.0;
    double p90 = 0.0;
    double p95 = 0.0;
    double max_error = 0.0;
    double confidence = 0.0;
    int sample_count = 0;
};

class AccuracyCalculator {
public:
    AccuracyReport calculate(const std::vector<ComparisonRecord>& records) const;

private:
    double computePercentile(std::vector<double> sorted_errors, double percentile) const;
};

} // namespace pa::spatial_truth