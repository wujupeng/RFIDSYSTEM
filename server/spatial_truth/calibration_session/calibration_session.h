#pragma once

#include "../ground_truth/ground_truth_point.h"
#include <string>
#include <cstdint>
#include <vector>

namespace pa {

enum class LocalizationMode {
    RSSI_ONLY = 0,
    RSSI_TRIANGULATION = 1,
    RSSI_PHASE = 2,
    AOA = 3,
    BEAMFORMING = 4,
};

} // namespace pa

namespace pa::spatial_truth {

enum class CalibrationSessionStatus {
    CREATED,
    RUNNING,
    ANALYZING,
    COMPLETED,
    FAILED,
};

struct CalibrationSession {
    std::string session_id;
    CalibrationSessionStatus status = CalibrationSessionStatus::CREATED;
    pa::LocalizationMode localization_mode = pa::LocalizationMode::RSSI_TRIANGULATION;
    std::string coordinate_system_id = "global";
    int ground_truth_count = 0;
    double target_accuracy = 0.1;
    uint64_t calibration_version = 0;
    uint64_t start_timestamp = 0;
    uint64_t completion_timestamp = 0;
    bool repeatability_verified = false;
};

struct ComparisonRecord {
    std::string record_id;
    std::string session_id;
    GroundTruthPoint ground_truth;
    double estimated_x = 0.0;
    double estimated_y = 0.0;
    double estimated_z = 0.0;
    double error_x = 0.0;
    double error_y = 0.0;
    double error_z = 0.0;
    double error_total = 0.0;
    bool skipped = false;
    std::string skip_reason;
};

inline bool canTransition(CalibrationSessionStatus from, CalibrationSessionStatus to) {
    if (to == CalibrationSessionStatus::FAILED) return true;
    switch (from) {
        case CalibrationSessionStatus::CREATED:    return to == CalibrationSessionStatus::RUNNING;
        case CalibrationSessionStatus::RUNNING:    return to == CalibrationSessionStatus::ANALYZING;
        case CalibrationSessionStatus::ANALYZING:  return to == CalibrationSessionStatus::COMPLETED;
        default: return false;
    }
}

inline const char* calibrationStatusToString(CalibrationSessionStatus s) {
    switch (s) {
        case CalibrationSessionStatus::CREATED:   return "created";
        case CalibrationSessionStatus::RUNNING:   return "running";
        case CalibrationSessionStatus::ANALYZING: return "analyzing";
        case CalibrationSessionStatus::COMPLETED: return "completed";
        case CalibrationSessionStatus::FAILED:    return "failed";
    }
    return "unknown";
}

} // namespace pa::spatial_truth