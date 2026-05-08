#include "auto_recalibration_engine.h"

namespace spatial_drift {

AutoRecalibrationEngine& AutoRecalibrationEngine::instance() {
    static AutoRecalibrationEngine engine;
    return engine;
}

void AutoRecalibrationEngine::initialize() {
    status_ = RecalibrationStatus::IDLE;
}

void AutoRecalibrationEngine::tick() {
    if (status_ == RecalibrationStatus::PENDING) {
        runRecalibration();
    }
    
    if (status_ == RecalibrationStatus::RUNNING) {
        current_task_.progress += 0.01f;
        if (current_task_.progress >= 1.0f) {
            completeRecalibration(true);
        }
    }
}

void AutoRecalibrationEngine::triggerRecalibration(const std::string& reason) {
    if (status_ == RecalibrationStatus::RUNNING) {
        return;
    }
    
    current_task_.task_id = task_history_.size() + 1;
    current_task_.status = RecalibrationStatus::PENDING;
    current_task_.trigger_timestamp = 0;
    current_task_.progress = 0.0f;
    current_task_.reason = reason;
    
    status_ = RecalibrationStatus::PENDING;
}

void AutoRecalibrationEngine::scheduleRecalibration(uint64_t delay_ms, const std::string& reason) {
    triggerRecalibration(reason);
}

RecalibrationStatus AutoRecalibrationEngine::getStatus() const {
    return status_;
}

const RecalibrationTask* AutoRecalibrationEngine::getCurrentTask() const {
    if (status_ == RecalibrationStatus::IDLE) {
        return nullptr;
    }
    return &current_task_;
}

void AutoRecalibrationEngine::cancelPendingRecalibration() {
    if (status_ == RecalibrationStatus::PENDING) {
        status_ = RecalibrationStatus::IDLE;
        current_task_.status = RecalibrationStatus::FAILED;
        task_history_.push_back(current_task_);
    }
}

bool AutoRecalibrationEngine::isRecalibrating() const {
    return status_ == RecalibrationStatus::RUNNING;
}

void AutoRecalibrationEngine::runRecalibration() {
    status_ = RecalibrationStatus::RUNNING;
    current_task_.status = RecalibrationStatus::RUNNING;
    current_task_.start_timestamp = 0;
    current_task_.progress = 0.0f;
}

void AutoRecalibrationEngine::completeRecalibration(bool success) {
    current_task_.status = success ? RecalibrationStatus::COMPLETED : RecalibrationStatus::FAILED;
    current_task_.completion_timestamp = 0;
    status_ = RecalibrationStatus::IDLE;
    
    task_history_.push_back(current_task_);
}

} // namespace spatial_drift