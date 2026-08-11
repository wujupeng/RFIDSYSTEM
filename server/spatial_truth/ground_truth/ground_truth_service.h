#pragma once

#include "ground_truth_point.h"
#include "ground_truth_store.h"
#include <vector>
#include <string>
#include <optional>

namespace pa::spatial_truth {

enum class GroundTruthError {
    NONE,
    FIELD_MISSING,
    INVALID_ACCURACY,
    INVALID_SOURCE,
    COORDINATE_SYSTEM_NOT_REGISTERED,
    POINT_REFERENCED_BY_CALIBRATION,
    POINT_NOT_FOUND,
    ACCURACY_CONFLICT,
};

struct AddGroundTruthResult {
    GroundTruthError error = GroundTruthError::NONE;
    std::string error_message;
    std::string point_id;
    bool conflict = false;
    std::string existing_point_id;
    double existing_accuracy = 0.0;
};

class GroundTruthService {
public:
    static GroundTruthService& instance();

    AddGroundTruthResult addGroundTruth(const GroundTruthPoint& point);
    std::vector<GroundTruthPoint> listGroundTruth(
        const std::optional<std::string>& tag_id = std::nullopt,
        const std::optional<std::string>& cs_id = std::nullopt,
        const std::optional<GroundTruthSource>& source = std::nullopt,
        int offset = 0,
        int limit = 100
    ) const;

    GroundTruthError deleteGroundTruth(const std::string& point_id);
    bool hasGroundTruth(const std::string& coordinate_system_id = "global") const;

    std::vector<GroundTruthPoint> getPointsForCalibration(
        const std::string& coordinate_system_id,
        const std::optional<GroundTruthSource>& source_filter = std::nullopt
    ) const;

private:
    GroundTruthService() = default;

    bool validateFields(const GroundTruthPoint& point, std::string& msg) const;
    bool isCoordinateSystemRegistered(const std::string& cs_id) const;
    bool isReferencedByCalibration(const std::string& point_id) const;
};

} // namespace pa::spatial_truth