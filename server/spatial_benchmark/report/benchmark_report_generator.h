#pragma once

#include "../scenarios/benchmark_scenario.h"
#include "../runner/scenario_runner.h"
#include "../../spatial_truth/calibration_session/accuracy_calculator.h"
#include "../../spatial_truth/ground_truth/ground_truth_point.h"
#include <string>
#include <vector>
#include <map>
#include <cstdint>

namespace pa::spatial_benchmark {

struct BenchmarkCell {
    double p50 = 0.0;
    double p90 = 0.0;
    double p95 = 0.0;
    double max_error = 0.0;
    bool na = false;
    std::string na_reason;
};

struct BenchmarkRow {
    pa::LocalizationMode algorithm;
    std::map<std::string, BenchmarkCell> cells;
};

struct BenchmarkComparisonTable {
    std::vector<BenchmarkRow> rows;
};

struct BenchmarkReport {
    std::string report_id;
    std::vector<BenchmarkScenario> scenarios;
    std::vector<pa::LocalizationMode> algorithms;
    BenchmarkComparisonTable comparison_table;
    uint64_t generated_at = 0;
};

enum class BenchmarkError {
    NONE,
    NO_GROUND_TRUTH,
    SCENARIO_NOT_DEFINED,
};

struct BenchmarkRunResult {
    BenchmarkError error = BenchmarkError::NONE;
    std::string error_message;
    BenchmarkReport report;
};

class BenchmarkReportGenerator {
public:
    BenchmarkRunResult generate(
        const std::vector<std::string>& scenario_ids,
        const std::vector<pa::LocalizationMode>& algorithms,
        const std::vector<pa::spatial_truth::GroundTruthPoint>& ground_truth_points
    );

    std::string exportJson(const BenchmarkReport& report) const;
    std::string exportCsv(const BenchmarkReport& report) const;

private:
    ScenarioRunner runner_;
    std::string generateReportId() const;
    std::string modeToString(pa::LocalizationMode m) const;
};

} // namespace pa::spatial_benchmark