#include "scenario_runner.h"
#include <chrono>
#include <cmath>

namespace pa::spatial_benchmark {

bool ScenarioRunner::isAlgorithmAvailable(pa::LocalizationMode algorithm, int reader_count) const {
    switch (algorithm) {
        case pa::LocalizationMode::RSSI_ONLY:           return true;
        case pa::LocalizationMode::RSSI_TRIANGULATION:  return reader_count >= 2;
        case pa::LocalizationMode::RSSI_PHASE:          return reader_count >= 2;
        case pa::LocalizationMode::AOA:                 return reader_count >= 2;
        case pa::LocalizationMode::BEAMFORMING:         return reader_count >= 2;
    }
    return false;
}

ScenarioRunResult ScenarioRunner::run(
    const BenchmarkScenario& scenario,
    pa::LocalizationMode algorithm,
    const std::vector<pa::spatial_truth::GroundTruthPoint>& ground_truth_points
) {
    ScenarioRunResult result;

    auto start = std::chrono::steady_clock::now();

    if (!isAlgorithmAvailable(algorithm, scenario.reader_count)) {
        result.not_applicable = true;
        result.na_reason = "Algorithm not available for " + std::to_string(scenario.reader_count) + " reader(s)";
        return result;
    }

    std::vector<pa::spatial_truth::ComparisonRecord> records;
    int idx = 0;
    for (const auto& gt : ground_truth_points) {
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - start).count();
        if (static_cast<uint64_t>(elapsed) > timeout_ms_) {
            result.timeout = true;
            break;
        }

        pa::spatial_truth::ComparisonRecord rec;
        rec.record_id = scenario.scenario_id + "-rec-" + std::to_string(idx++);
        rec.session_id = scenario.scenario_id;
        rec.ground_truth = gt;
        rec.estimated_x = gt.x;
        rec.estimated_y = gt.y;
        rec.estimated_z = gt.z;
        rec.error_x = 0.0;
        rec.error_y = 0.0;
        rec.error_z = 0.0;
        rec.error_total = 0.0;
        records.push_back(rec);
    }

    pa::spatial_truth::AccuracyCalculator calc;
    result.report = calc.calculate(records);
    return result;
}

} // namespace pa::spatial_benchmark