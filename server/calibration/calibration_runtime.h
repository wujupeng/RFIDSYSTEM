#pragma once

#include <cstdint>
#include <vector>
#include <map>
#include "calibration_snapshot.h"

namespace calibration {

enum class CalibrationStatus {
    IDLE,
    CALIBRATING,
    RECALIBRATING,
    VALIDATING,
    DEGRADED
};

struct CalibrationTask {
    uint64_t task_id;
    uint64_t reader_id;
    uint64_t antenna_id;
    CalibrationStatus status;
    double progress;
    uint64_t start_timestamp;
};

class CalibrationRuntime {
public:
    static CalibrationRuntime& instance();
    
    void initialize();
    
    void shutdown();
    
    void tick();
    
    void triggerFullCalibration();
    
    void triggerReaderCalibration(uint64_t reader_id);
    
    void triggerAntennaCalibration(uint64_t reader_id, uint64_t antenna_id);
    
    void triggerRecalibration();
    
    CalibrationStatus getStatus() const;
    
    uint64_t getCurrentVersion() const;
    
    bool isCalibrated() const;
    
    void validateCalibration();
    
    void applyCalibrationForFrame(uint64_t frame_timestamp);
    
private:
    CalibrationRuntime() = default;
    
    void runCalibrationSequence();
    
    void updateCalibrationStatus();
    
    void saveSnapshotAfterCalibration();
    
    CalibrationStatus status_ = CalibrationStatus::IDLE;
    uint64_t current_version_ = 0;
    std::vector<CalibrationTask> active_tasks_;
};

} // namespace calibration