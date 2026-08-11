#pragma once

#include "../../spatial_truth/calibration_session/calibration_session.h"
#include <string>
#include <vector>

namespace pa::spatial_benchmark {

struct BenchmarkScenario {
    std::string scenario_id;
    std::string name;
    std::string description;
    std::vector<pa::LocalizationMode> algorithms;
    int reader_count = 1;
    bool is_preset = true;
};

} // namespace pa::spatial_benchmark