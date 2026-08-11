#include "benchmark_report_generator.h"
#include <chrono>
#include <sstream>
#include <cmath>

namespace pa::spatial_benchmark {

std::string BenchmarkReportGenerator::generateReportId() const {
    auto now = std::chrono::steady_clock::now().time_since_epoch();
    auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(now).count();
    return "bench-" + std::to_string(ns);
}

std::string BenchmarkReportGenerator::modeToString(pa::LocalizationMode m) const {
    switch (m) {
        case pa::LocalizationMode::RSSI_ONLY:          return "rssi_only";
        case pa::LocalizationMode::RSSI_TRIANGULATION: return "rssi_triangulation";
        case pa::LocalizationMode::RSSI_PHASE:         return "rssi_phase";
        case pa::LocalizationMode::AOA:                return "aoa";
        case pa::LocalizationMode::BEAMFORMING:        return "beamforming";
    }
    return "unknown";
}

BenchmarkRunResult BenchmarkReportGenerator::generate(
    const std::vector<std::string>& scenario_ids,
    const std::vector<pa::LocalizationMode>& algorithms,
    const std::vector<pa::spatial_truth::GroundTruthPoint>& ground_truth_points
) {
    BenchmarkRunResult result;

    if (ground_truth_points.empty()) {
        result.error = BenchmarkError::NO_GROUND_TRUTH;
        result.error_message = "No ground truth points available";
        return result;
    }

    auto& registry = BenchmarkScenarioRegistry::instance();

    BenchmarkReport report;
    report.report_id = generateReportId();
    report.algorithms = algorithms;
    report.generated_at = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();

    for (const auto& sid : scenario_ids) {
        auto scenario = registry.getScenario(sid);
        if (!scenario) {
            result.error = BenchmarkError::SCENARIO_NOT_DEFINED;
            result.error_message = "Scenario not defined: " + sid;
            return result;
        }
        report.scenarios.push_back(*scenario);
    }

    for (auto algo : algorithms) {
        BenchmarkRow row;
        row.algorithm = algo;

        for (const auto& scenario : report.scenarios) {
            auto run_result = runner_.run(scenario, algo, ground_truth_points);

            BenchmarkCell cell;
            if (run_result.not_applicable) {
                cell.na = true;
                cell.na_reason = run_result.na_reason;
            } else {
                cell.p50 = run_result.report.p50;
                cell.p90 = run_result.report.p90;
                cell.p95 = run_result.report.p95;
                cell.max_error = run_result.report.max_error;
            }
            row.cells[scenario.scenario_id] = cell;
        }
        report.comparison_table.rows.push_back(row);
    }

    result.report = report;
    return result;
}

std::string BenchmarkReportGenerator::exportJson(const BenchmarkReport& report) const {
    std::ostringstream ss;
    ss << "{\"report_id\":\"" << report.report_id << "\",";
    ss << "\"generated_at\":" << report.generated_at << ",";
    ss << "\"scenarios\":[";
    for (size_t i = 0; i < report.scenarios.size(); ++i) {
        if (i > 0) ss << ",";
        ss << "{\"id\":\"" << report.scenarios[i].scenario_id << "\",\"name\":\"" << report.scenarios[i].name << "\"}";
    }
    ss << "],\"algorithms\":[";
    for (size_t i = 0; i < report.algorithms.size(); ++i) {
        if (i > 0) ss << ",";
        ss << "\"" << modeToString(report.algorithms[i]) << "\"";
    }
    ss << "],\"comparison_table\":{\"rows\":[";
    for (size_t i = 0; i < report.comparison_table.rows.size(); ++i) {
        if (i > 0) ss << ",";
        const auto& row = report.comparison_table.rows[i];
        ss << "{\"algorithm\":\"" << modeToString(row.algorithm) << "\",\"cells\":{";
        bool first = true;
        for (const auto& [sid, cell] : row.cells) {
            if (!first) ss << ",";
            first = false;
            ss << "\"" << sid << "\":{";
            if (cell.na) {
                ss << "\"na\":true,\"reason\":\"" << cell.na_reason << "\"";
            } else {
                ss << "\"p50\":" << cell.p50 << ",\"p90\":" << cell.p90
                   << ",\"p95\":" << cell.p95 << ",\"max_error\":" << cell.max_error;
            }
            ss << "}";
        }
        ss << "}}";
    }
    ss << "]}}";
    return ss.str();
}

std::string BenchmarkReportGenerator::exportCsv(const BenchmarkReport& report) const {
    std::ostringstream ss;
    ss << "algorithm,scenario,p50,p90,p95,max_error,na\n";
    for (const auto& row : report.comparison_table.rows) {
        for (const auto& [sid, cell] : row.cells) {
            ss << modeToString(row.algorithm) << "," << sid << ",";
            if (cell.na) {
                ss << ",,,," << cell.na_reason << "\n";
            } else {
                ss << cell.p50 << "," << cell.p90 << "," << cell.p95 << "," << cell.max_error << ",\n";
            }
        }
    }
    return ss.str();
}

} // namespace pa::spatial_benchmark