#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace pa::spatial_truth {

struct ObservationData {
    std::string epc;
    double rssi = 0.0;
    double phase = 0.0;
    std::string reader_id;
    uint64_t timestamp = 0;
};

struct RFEnvironmentFeatures {
    double channel_quality = 0.0;
    double interference_level = 0.0;
    double multipath_factor = 0.0;
};

class SourceHashCalculator {
public:
    uint64_t calculate(const std::vector<ObservationData>& observations) const;
    uint64_t calculateEnvironmentHash(const RFEnvironmentFeatures& rf_features) const;
};

} // namespace pa::spatial_truth