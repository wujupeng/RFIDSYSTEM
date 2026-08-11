#pragma once

#include <string>
#include <cstdint>
#include <vector>

namespace pa::spatial_truth {

enum class CoordinateUnit {
    METER,
    CENTIMETER,
    MILLIMETER
};

struct SpatialCoordinateSystem {
    std::string id;
    std::string parent_id;
    double origin_x = 0.0;
    double origin_y = 0.0;
    double origin_z = 0.0;
    double rotation = 0.0;
    CoordinateUnit unit = CoordinateUnit::METER;
    uint64_t version = 1;
};

struct Point3D {
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
};

inline const char* coordinateUnitToString(CoordinateUnit u) {
    switch (u) {
        case CoordinateUnit::METER:      return "meter";
        case CoordinateUnit::CENTIMETER: return "centimeter";
        case CoordinateUnit::MILLIMETER: return "millimeter";
    }
    return "unknown";
}

inline CoordinateUnit stringToCoordinateUnit(const std::string& s) {
    if (s == "meter")      return CoordinateUnit::METER;
    if (s == "centimeter") return CoordinateUnit::CENTIMETER;
    if (s == "millimeter") return CoordinateUnit::MILLIMETER;
    return CoordinateUnit::METER;
}

} // namespace pa::spatial_truth