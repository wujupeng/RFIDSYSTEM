#include "coordinate_transformer.h"
#include <cmath>
#include <algorithm>

namespace pa::spatial_truth {

double CoordinateTransformer::toMeters(double value, CoordinateUnit unit) const {
    switch (unit) {
        case CoordinateUnit::METER:      return value;
        case CoordinateUnit::CENTIMETER: return value / 100.0;
        case CoordinateUnit::MILLIMETER: return value / 1000.0;
    }
    return value;
}

double CoordinateTransformer::fromMeters(double value, CoordinateUnit unit) const {
    switch (unit) {
        case CoordinateUnit::METER:      return value;
        case CoordinateUnit::CENTIMETER: return value * 100.0;
        case CoordinateUnit::MILLIMETER: return value * 1000.0;
    }
    return value;
}

Point3D CoordinateTransformer::applyTransform(const Point3D& p, const SpatialCoordinateSystem& cs) const {
    Point3D result;
    double px = toMeters(p.x, cs.unit);
    double py = toMeters(p.y, cs.unit);
    double pz = toMeters(p.z, cs.unit);

    double cos_r = std::cos(cs.rotation);
    double sin_r = std::sin(cs.rotation);

    result.x = cs.origin_x + cos_r * px - sin_r * py;
    result.y = cs.origin_y + sin_r * px + cos_r * py;
    result.z = cs.origin_z + pz;
    return result;
}

Point3D CoordinateTransformer::applyInverseTransform(const Point3D& p, const SpatialCoordinateSystem& cs) const {
    Point3D result;
    double dx = p.x - cs.origin_x;
    double dy = p.y - cs.origin_y;
    double dz = p.z - cs.origin_z;

    double cos_r = std::cos(-cs.rotation);
    double sin_r = std::sin(-cs.rotation);

    double rx = cos_r * dx - sin_r * dy;
    double ry = sin_r * dx + cos_r * dy;

    result.x = fromMeters(rx, cs.unit);
    result.y = fromMeters(ry, cs.unit);
    result.z = fromMeters(dz, cs.unit);
    return result;
}

std::vector<std::string> CoordinateTransformer::getPathToRoot(const std::string& cs_id) const {
    std::vector<std::string> path;
    std::string current = cs_id;
    int max_depth = 10;

    while (!current.empty() && max_depth-- > 0) {
        auto cs = CoordinateSystemManager::instance().get(current);
        if (!cs) break;
        path.push_back(current);
        if (cs->parent_id.empty()) break;
        current = cs->parent_id;
    }
    return path;
}

TransformResult CoordinateTransformer::transform(const Point3D& point,
                                                   const std::string& source_cs_id,
                                                   const std::string& target_cs_id) const {
    TransformResult result;

    if (source_cs_id == target_cs_id) {
        result.result = point;
        return result;
    }

    auto source_cs = CoordinateSystemManager::instance().get(source_cs_id);
    auto target_cs = CoordinateSystemManager::instance().get(target_cs_id);
    if (!source_cs || !target_cs) {
        result.error = TransformError::COORDINATE_SYSTEM_NOT_FOUND;
        result.error_message = "Coordinate system not found";
        return result;
    }

    auto source_path = getPathToRoot(source_cs_id);
    auto target_path = getPathToRoot(target_cs_id);

    Point3D current = point;

    for (const auto& cs_id : source_path) {
        if (cs_id == source_cs_id) continue;
        auto cs = CoordinateSystemManager::instance().get(cs_id);
        if (!cs) {
            result.error = TransformError::TRANSFORM_CHAIN_BROKEN;
            result.error_message = "Transform chain broken at: " + cs_id;
            return result;
        }
        current = applyTransform(current, *cs);
    }

    for (auto it = target_path.rbegin(); it != target_path.rend(); ++it) {
        auto cs = CoordinateSystemManager::instance().get(*it);
        if (!cs) {
            result.error = TransformError::TRANSFORM_CHAIN_BROKEN;
            result.error_message = "Transform chain broken at: " + *it;
            return result;
        }
        current = applyInverseTransform(current, *cs);
    }

    result.result = current;
    return result;
}

TransformResult CoordinateTransformer::transformChain(const Point3D& point,
                                                        const std::vector<std::string>& chain) const {
    TransformResult result;
    Point3D current = point;

    for (size_t i = 0; i + 1 < chain.size(); ++i) {
        auto tr = transform(current, chain[i], chain[i + 1]);
        if (tr.error != TransformError::NONE) return tr;
        current = tr.result;
    }

    result.result = current;
    return result;
}

bool CoordinateTransformer::validateRoundTrip(const Point3D& point,
                                               const std::string& source_cs_id,
                                               const std::string& target_cs_id) const {
    auto forward = transform(point, source_cs_id, target_cs_id);
    if (forward.error != TransformError::NONE) return false;

    auto backward = transform(forward.result, target_cs_id, source_cs_id);
    if (backward.error != TransformError::NONE) return false;

    double dx = point.x - backward.result.x;
    double dy = point.y - backward.result.y;
    double dz = point.z - backward.result.z;
    double dist = std::sqrt(dx * dx + dy * dy + dz * dz);

    return dist <= 0.001;
}

} // namespace pa::spatial_truth