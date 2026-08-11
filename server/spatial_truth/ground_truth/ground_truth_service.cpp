#include "ground_truth_service.h"

namespace pa::spatial_truth {

GroundTruthService& GroundTruthService::instance() {
    static GroundTruthService inst;
    return inst;
}

bool GroundTruthService::validateFields(const GroundTruthPoint& point, std::string& msg) const {
    if (point.point_id.empty()) { msg = "point_id is required"; return false; }
    if (point.tag_id.empty()) { msg = "tag_id is required"; return false; }
    if (point.coordinate_system_id.empty()) { msg = "coordinate_system_id is required"; return false; }
    if (point.timestamp == 0) { msg = "timestamp is required"; return false; }
    if (point.accuracy < 0) { msg = "accuracy must be non-negative"; return false; }
    return true;
}

bool GroundTruthService::isCoordinateSystemRegistered(const std::string& cs_id) const {
    if (cs_id == "global") return true;
    return true;
}

bool GroundTruthService::isReferencedByCalibration(const std::string& point_id) const {
    return false;
}

AddGroundTruthResult GroundTruthService::addGroundTruth(const GroundTruthPoint& point) {
    AddGroundTruthResult result;

    std::string msg;
    if (!validateFields(point, msg)) {
        result.error = GroundTruthError::FIELD_MISSING;
        result.error_message = msg;
        return result;
    }

    if (!isCoordinateSystemRegistered(point.coordinate_system_id)) {
        result.error = GroundTruthError::COORDINATE_SYSTEM_NOT_REGISTERED;
        result.error_message = "Coordinate system not registered: " + point.coordinate_system_id;
        return result;
    }

    DedupResult dedup;
    GroundTruthPoint p = point;
    if (p.created_at == 0) {
        p.created_at = p.timestamp;
    }

    bool success = GroundTruthStore::instance().addPoint(p, &dedup);
    if (!success && dedup.conflict) {
        result.error = GroundTruthError::ACCURACY_CONFLICT;
        result.error_message = "Higher accuracy point exists for same tag_id within dedup window";
        result.conflict = true;
        result.existing_point_id = dedup.existing_point_id;
        result.existing_accuracy = dedup.existing_accuracy;
        return result;
    }

    result.point_id = p.point_id;
    return result;
}

std::vector<GroundTruthPoint> GroundTruthService::listGroundTruth(
    const std::optional<std::string>& tag_id,
    const std::optional<std::string>& cs_id,
    const std::optional<GroundTruthSource>& source,
    int offset,
    int limit
) const {
    std::vector<GroundTruthPoint> all;

    if (tag_id && cs_id) {
        all = GroundTruthStore::instance().listByTagIdAndCoordinateSystem(*tag_id, *cs_id);
    } else if (tag_id) {
        all = GroundTruthStore::instance().listByTagId(*tag_id);
    } else if (cs_id) {
        all = GroundTruthStore::instance().listByCoordinateSystem(*cs_id);
    } else {
        all = GroundTruthStore::instance().listByTimeRange(0, UINT64_MAX);
    }

    if (source) {
        std::vector<GroundTruthPoint> filtered;
        for (const auto& p : all) {
            if (p.source == *source) filtered.push_back(p);
        }
        all = std::move(filtered);
    }

    if (offset < 0) offset = 0;
    if (limit < 0) limit = 100;

    std::vector<GroundTruthPoint> result;
    for (int i = offset; i < (int)all.size() && (int)result.size() < limit; ++i) {
        result.push_back(all[i]);
    }
    return result;
}

GroundTruthError GroundTruthService::deleteGroundTruth(const std::string& point_id) {
    auto point = GroundTruthStore::instance().getPoint(point_id);
    if (!point) {
        return GroundTruthError::POINT_NOT_FOUND;
    }

    if (isReferencedByCalibration(point_id)) {
        return GroundTruthError::POINT_REFERENCED_BY_CALIBRATION;
    }

    GroundTruthStore::instance().removePoint(point_id);
    return GroundTruthError::NONE;
}

bool GroundTruthService::hasGroundTruth(const std::string& coordinate_system_id) const {
    auto points = GroundTruthStore::instance().listByCoordinateSystem(coordinate_system_id);
    return !points.empty();
}

std::vector<GroundTruthPoint> GroundTruthService::getPointsForCalibration(
    const std::string& coordinate_system_id,
    const std::optional<GroundTruthSource>& source_filter
) const {
    auto points = GroundTruthStore::instance().listByCoordinateSystem(coordinate_system_id);
    if (source_filter) {
        std::vector<GroundTruthPoint> filtered;
        for (const auto& p : points) {
            if (p.source == *source_filter) filtered.push_back(p);
        }
        return filtered;
    }
    return points;
}

} // namespace pa::spatial_truth