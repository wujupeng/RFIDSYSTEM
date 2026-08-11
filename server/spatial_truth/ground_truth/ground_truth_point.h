#pragma once

#include <string>
#include <cstdint>

namespace pa::spatial_truth {

enum class GroundTruthSource {
    MANUAL,
    TOTAL_STATION,
    LASER,
    REFERENCE_POINT,
    KNOWN_TAG
};

struct GroundTruthPoint {
    std::string point_id;
    std::string tag_id;
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
    uint64_t timestamp = 0;
    std::string coordinate_system_id = "global";
    double accuracy = 0.0;
    GroundTruthSource source = GroundTruthSource::MANUAL;
    uint64_t created_at = 0;
};

inline const char* groundTruthSourceToString(GroundTruthSource s) {
    switch (s) {
        case GroundTruthSource::MANUAL:           return "manual";
        case GroundTruthSource::TOTAL_STATION:    return "total_station";
        case GroundTruthSource::LASER:            return "laser";
        case GroundTruthSource::REFERENCE_POINT:  return "reference_point";
        case GroundTruthSource::KNOWN_TAG:        return "known_tag";
    }
    return "unknown";
}

inline GroundTruthSource stringToGroundTruthSource(const std::string& s) {
    if (s == "manual")           return GroundTruthSource::MANUAL;
    if (s == "total_station")    return GroundTruthSource::TOTAL_STATION;
    if (s == "laser")            return GroundTruthSource::LASER;
    if (s == "reference_point")  return GroundTruthSource::REFERENCE_POINT;
    if (s == "known_tag")        return GroundTruthSource::KNOWN_TAG;
    return GroundTruthSource::MANUAL;
}

} // namespace pa::spatial_truth