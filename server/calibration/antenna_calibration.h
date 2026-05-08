#pragma once

#include <cstdint>
#include <vector>
#include <map>

namespace calibration {

struct AntennaCalibration {
    uint64_t antenna_id;
    uint64_t reader_id;
    float gain;
    float azimuth_offset_deg;
    float elevation_offset_deg;
    float beamwidth_azimuth_deg;
    float beamwidth_elevation_deg;
    double last_calibrated_timestamp;
};

class AntennaCalibrationManager {
public:
    static AntennaCalibrationManager& instance();
    
    void calibrateAntenna(uint64_t reader_id, uint64_t antenna_id);
    
    const AntennaCalibration* getCalibration(uint64_t reader_id, uint64_t antenna_id) const;
    
    void updateCalibration(uint64_t reader_id, uint64_t antenna_id, const AntennaCalibration& calib);
    
    void loadCalibrations(const std::vector<AntennaCalibration>& calibrations);
    
    std::vector<AntennaCalibration> exportCalibrations() const;
    
    float correctAzimuth(uint64_t reader_id, uint64_t antenna_id, float raw_azimuth) const;
    
    float correctElevation(uint64_t reader_id, uint64_t antenna_id, float raw_elevation) const;
    
private:
    AntennaCalibrationManager() = default;
    
    std::map<std::pair<uint64_t, uint64_t>, AntennaCalibration> calibrations_;
};

} // namespace calibration