#pragma once

#include "benchmark_scenario.h"
#include <vector>
#include <optional>

namespace pa::spatial_benchmark {

class BenchmarkScenarioRegistry {
public:
    static BenchmarkScenarioRegistry& instance();

    std::optional<BenchmarkScenario> getScenario(const std::string& id) const;
    std::vector<BenchmarkScenario> listScenarios() const;
    bool isPreset(const std::string& id) const;

private:
    BenchmarkScenarioRegistry();
    std::vector<BenchmarkScenario> scenarios_;
};

} // namespace pa::spatial_benchmark