#include "benchmark_scenario_registry.h"

namespace pa::spatial_benchmark {

BenchmarkScenarioRegistry& BenchmarkScenarioRegistry::instance() {
    static BenchmarkScenarioRegistry inst;
    return inst;
}

BenchmarkScenarioRegistry::BenchmarkScenarioRegistry() {
    scenarios_ = {
        {"A", "Single Reader RSSI",
         "Single reader, RSSI-only localization",
         {pa::LocalizationMode::RSSI_ONLY}, 1, true},

        {"B", "Multi Reader RSSI Triangulation",
         "Multiple readers, RSSI + triangulation",
         {pa::LocalizationMode::RSSI_ONLY, pa::LocalizationMode::RSSI_TRIANGULATION}, 3, true},

        {"C", "Phase-based Localization",
         "Multiple readers, RSSI + Phase",
         {pa::LocalizationMode::RSSI_ONLY, pa::LocalizationMode::RSSI_PHASE}, 3, true},

        {"D", "AoA Localization",
         "Multiple readers, AoA + triangulation",
         {pa::LocalizationMode::AOA, pa::LocalizationMode::RSSI_TRIANGULATION}, 4, true},

        {"E", "Beamforming",
         "Phased array beamforming",
         {pa::LocalizationMode::BEAMFORMING}, 4, true},
    };
}

std::optional<BenchmarkScenario> BenchmarkScenarioRegistry::getScenario(const std::string& id) const {
    for (const auto& s : scenarios_) {
        if (s.scenario_id == id) return s;
    }
    return std::nullopt;
}

std::vector<BenchmarkScenario> BenchmarkScenarioRegistry::listScenarios() const {
    return scenarios_;
}

bool BenchmarkScenarioRegistry::isPreset(const std::string& id) const {
    for (const auto& s : scenarios_) {
        if (s.scenario_id == id) return s.is_preset;
    }
    return false;
}

} // namespace pa::spatial_benchmark