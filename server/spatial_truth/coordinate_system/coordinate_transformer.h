#pragma once

#include "spatial_coordinate_system.h"
#include "coordinate_system_manager.h"
#include <vector>
#include <string>
#include <optional>

namespace pa::spatial_truth {

enum class TransformError {
    NONE,
    COORDINATE_SYSTEM_NOT_FOUND,
    TRANSFORM_CHAIN_BROKEN,
};

struct TransformResult {
    TransformError error = TransformError::NONE;
    std::string error_message;
    Point3D result;
};

class CoordinateTransformer {
public:
    TransformResult transform(const Point3D& point,
                              const std::string& source_cs_id,
                              const std::string& target_cs_id) const;

    TransformResult transformChain(const Point3D& point,
                                   const std::vector<std::string>& chain) const;

    bool validateRoundTrip(const Point3D& point,
                           const std::string& source_cs_id,
                           const std::string& target_cs_id) const;

private:
    Point3D applyTransform(const Point3D& p, const SpatialCoordinateSystem& cs) const;
    Point3D applyInverseTransform(const Point3D& p, const SpatialCoordinateSystem& cs) const;
    double toMeters(double value, CoordinateUnit unit) const;
    double fromMeters(double value, CoordinateUnit unit) const;
    std::vector<std::string> getPathToRoot(const std::string& cs_id) const;
};

} // namespace pa::spatial_truth