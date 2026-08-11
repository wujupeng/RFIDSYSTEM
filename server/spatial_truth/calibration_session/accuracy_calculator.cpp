#include "accuracy_calculator.h"
#include <algorithm>
#include <cmath>
#include <numeric>

namespace pa::spatial_truth {

double AccuracyCalculator::computePercentile(std::vector<double> sorted_errors, double percentile) const {
    if (sorted_errors.empty()) return 0.0;
    if (sorted_errors.size() == 1) return sorted_errors[0];

    std::sort(sorted_errors.begin(), sorted_errors.end());

    double rank = percentile / 100.0 * (sorted_errors.size() - 1);
    size_t lo = static_cast<size_t>(std::floor(rank));
    size_t hi = static_cast<size_t>(std::ceil(rank));
    double frac = rank - lo;

    return sorted_errors[lo] * (1.0 - frac) + sorted_errors[hi] * frac;
}

AccuracyReport AccuracyCalculator::calculate(const std::vector<ComparisonRecord>& records) const {
    AccuracyReport report;

    std::vector<ComparisonRecord> valid;
    for (const auto& r : records) {
        if (!r.skipped) valid.push_back(r);
    }

    report.sample_count = static_cast<int>(valid.size());
    if (valid.empty()) return report;

    std::vector<double> errors;
    errors.reserve(valid.size());
    double sum_error = 0.0;
    double sum_error_sq = 0.0;
    double sum_abs_x = 0.0;
    double sum_abs_y = 0.0;
    double sum_abs_total = 0.0;
    double max_err = 0.0;
    double sum_confidence = 0.0;

    for (const auto& r : valid) {
        double err = r.error_total;
        errors.push_back(err);
        sum_error += err;
        sum_error_sq += err * err;
        sum_abs_x += std::abs(r.error_x);
        sum_abs_y += std::abs(r.error_y);
        sum_abs_total += std::abs(err);
        if (err > max_err) max_err = err;
        sum_confidence += 1.0;
    }

    int n = report.sample_count;

    report.position_error = sum_abs_total / n;
    report.x_error = sum_abs_x / n;
    report.y_error = sum_abs_y / n;
    report.rmse = std::sqrt(sum_error_sq / n);
    report.mae = sum_abs_total / n;
    report.max_error = max_err;

    report.p50 = computePercentile(errors, 50.0);
    report.p90 = computePercentile(errors, 90.0);
    report.p95 = computePercentile(errors, 95.0);

    report.confidence = sum_confidence / n;

    return report;
}

} // namespace pa::spatial_truth