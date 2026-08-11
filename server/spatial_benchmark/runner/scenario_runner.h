#pragma once

#include "../scenarios/benchmark_scenario.h"
#include "../../spatial_truth/calibration_session/accuracy_calculator.h"
#include "../../spatial_truth/ground_truth/ground_truth_point.h"
#include <optional>

namespace pa::spatial_benchmark {

struct ScenarioRunResult {
    pa::spatial_truth::AccuracyReport report;
    bool timeout = false;
    bool not_applicable = false;
    std::string na_reason;
};

class ScenarioRunner {
public:
    ScenarioRunResult run(
        const BenchmarkScenario& scenario,
        pa::LocalizationMode algorithm,
        const std::vector<pa::spatial_truth::GroundTruthPoint>& ground_truth_points
    );

    void setTimeoutMs(uint64_t timeout_ms) { timeout_ms_ = timeout_ms; }

private:
    uint64_t timeout_ms_ = 60000;
    bool isAlgorithmAvailable(pa::LocalizationMode algorithm, int reader_count) const;
};

} // namespace pa::spatial_benchmark