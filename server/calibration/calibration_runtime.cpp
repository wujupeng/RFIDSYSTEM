#include "calibration_runtime.h"
#include "calibration_snapshot.h"

namespace calibration {

CalibrationRuntime& CalibrationRuntime::instance() {
    static CalibrationRuntime runtime;
    return runtime;
}

void CalibrationRuntime::initialize() {
    status_ = CalibrationStatus::IDLE;
    current_version_ = 0;
}

void CalibrationRuntime::shutdown() {
    active_tasks_.clear();
}

void CalibrationRuntime::tick() {
    updateCalibrationStatus();
}

void CalibrationRuntime::triggerFullCalibration() {
    if (status_ == CalibrationStatus::CALIBRATING || 
        status_ == CalibrationStatus::RECALIBRATING) {
        return;
    }
    
    status_ = CalibrationStatus::CALIBRATING;
    runCalibrationSequence();
}

void CalibrationRuntime::triggerReaderCalibration(uint64_t reader_id) {
    CalibrationTask task;
    task.task_id = reader_id;
    task.reader_id = reader_id;
    task.antenna_id = 0;
    task.status = CalibrationStatus::CALIBRATING;
    task.progress = 0.0;
    task.start_timestamp = 0;
    
    active_tasks_.push_back(task);
}

void CalibrationRuntime::triggerAntennaCalibration(uint64_t reader_id, uint64_t antenna_id) {
    CalibrationTask task;
    task.task_id = (reader_id << 32) | antenna_id;
    task.reader_id = reader_id;
    task.antenna_id = antenna_id;
    task.status = CalibrationStatus::CALIBRATING;
    task.progress = 0.0;
    task.start_timestamp = 0;
    
    active_tasks_.push_back(task);
}

void CalibrationRuntime::triggerRecalibration() {
    if (status_ == CalibrationStatus::CALIBRATING ||
        status_ == CalibrationStatus::RECALIBRATING) {
        return;
    }
    
    status_ = CalibrationStatus::RECALIBRATING;
    runCalibrationSequence();
}

CalibrationStatus CalibrationRuntime::getStatus() const {
    return status_;
}

uint64_t CalibrationRuntime::getCurrentVersion() const {
    return current_version_;
}

bool CalibrationRuntime::isCalibrated() const {
    return current_version_ > 0 && status_ != CalibrationStatus::DEGRADED;
}

void CalibrationRuntime::validateCalibration() {
    status_ = CalibrationStatus::VALIDATING;
    status_ = CalibrationStatus::IDLE;
}

void CalibrationRuntime::applyCalibrationForFrame(uint64_t frame_timestamp) {
    CalibrationSnapshotManager::instance().loadSnapshotByTimestamp(frame_timestamp);
}

void CalibrationRuntime::runCalibrationSequence() {
    active_tasks_.clear();
    
    saveSnapshotAfterCalibration();
    
    status_ = CalibrationStatus::IDLE;
}

void CalibrationRuntime::updateCalibrationStatus() {
    for (auto& task : active_tasks_) {
        if (task.status == CalibrationStatus::CALIBRATING) {
            task.progress += 0.1;
            if (task.progress >= 1.0) {
                task.status = CalibrationStatus::VALIDATING;
            }
        }
    }
}

void CalibrationRuntime::saveSnapshotAfterCalibration() {
    CalibrationSnapshotManager::instance().createSnapshot("Auto-calibration");
    current_version_ = CalibrationSnapshotManager::instance().getCurrentSnapshot()->version;
}

} // namespace calibration