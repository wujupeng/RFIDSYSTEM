#include "antenna_calibration.h"

namespace calibration {

AntennaCalibrationManager& AntennaCalibrationManager::instance() {
    static AntennaCalibrationManager manager;
    return manager;
}

void AntennaCalibrationManager::calibrateAntenna(uint64_t reader_id, uint64_t antenna_id) {
    AntennaCalibration calib;
    calib.antenna_id = antenna_id;
    calib.reader_id = reader_id;
    calib.gain = 1.0f;
    calib.azimuth_offset_deg = 0.0f;
    calib.elevation_offset_deg = 0.0f;
    calib.beamwidth_azimuth_deg = 60.0f;
    calib.beamwidth_elevation_deg = 30.0f;
    calib.last_calibrated_timestamp = 0;
    
    calibrations_[{reader_id, antenna_id}] = calib;
}

const AntennaCalibration* AntennaCalibrationManager::getCalibration(uint64_t reader_id, uint64_t antenna_id) const {
    auto key = std::make_pair(reader_id, antenna_id);
    auto it = calibrations_.find(key);
    return (it != calibrations_.end()) ? &it->second : nullptr;
}

void AntennaCalibrationManager::updateCalibration(uint64_t reader_id, uint64_t antenna_id, const AntennaCalibration& calib) {
    calibrations_[{reader_id, antenna_id}] = calib;
}

void AntennaCalibrationManager::loadCalibrations(const std::vector<AntennaCalibration>& calibrations) {
    for (const auto& calib : calibrations) {
        calibrations_[{calib.reader_id, calib.antenna_id}] = calib;
    }
}

std::vector<AntennaCalibration> AntennaCalibrationManager::exportCalibrations() const {
    std::vector<AntennaCalibration> result;
    for (const auto& pair : calibrations_) {
        result.push_back(pair.second);
    }
    return result;
}

float AntennaCalibrationManager::correctAzimuth(uint64_t reader_id, uint64_t antenna_id, float raw_azimuth) const {
    const AntennaCalibration* calib = getCalibration(reader_id, antenna_id);
    if (!calib) {
        return raw_azimuth;
    }
    float corrected = raw_azimuth - calib->azimuth_offset_deg;
    while (corrected < 0.0f) corrected += 360.0f;
    while (corrected >= 360.0f) corrected -= 360.0f;
    return corrected;
}

float AntennaCalibrationManager::correctElevation(uint64_t reader_id, uint64_t antenna_id, float raw_elevation) const {
    const AntennaCalibration* calib = getCalibration(reader_id, antenna_id);
    if (!calib) {
        return raw_elevation;
    }
    return raw_elevation - calib->elevation_offset_deg;
}

} // namespace calibration