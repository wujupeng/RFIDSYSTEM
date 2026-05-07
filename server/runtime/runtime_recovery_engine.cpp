#include "runtime_recovery_engine.h"

RuntimeRecoveryEngine::RuntimeRecoveryEngine()
    : in_recovery_(false),
      recovery_count_(0),
      last_action_(RecoveryAction::NONE) {
}

RuntimeRecoveryEngine& RuntimeRecoveryEngine::instance() {
    static RuntimeRecoveryEngine instance;
    return instance;
}

void RuntimeRecoveryEngine::initialize() {
    in_recovery_ = false;
    recovery_count_ = 0;
    last_action_ = RecoveryAction::NONE;
    last_action_reason_ = "";
}

void RuntimeRecoveryEngine::shutdown() {
    std::lock_guard<std::mutex> lock(mutex_);
    in_recovery_ = false;
    recovery_callbacks_.clear();
    start_callbacks_.clear();
    end_callbacks_.clear();
}

RecoveryAction RuntimeRecoveryEngine::decide(const FailureEvent& event) {
    return mapFailureToAction(event.type);
}

bool RuntimeRecoveryEngine::execute(RecoveryAction action) {
    if (action == RecoveryAction::NONE) {
        return true;
    }
    
    beginRecovery(action, "");
    
    for (const auto& callback : recovery_callbacks_) {
        try {
            if (!callback(action)) {
                endRecovery(false);
                return false;
            }
        } catch (...) {
            endRecovery(false);
            return false;
        }
    }
    
    endRecovery(true);
    return true;
}

bool RuntimeRecoveryEngine::inRecovery() const {
    return in_recovery_;
}

size_t RuntimeRecoveryEngine::getRecoveryCount() const {
    return recovery_count_;
}

RecoveryAction RuntimeRecoveryEngine::getLastAction() const {
    return last_action_;
}

const std::string& RuntimeRecoveryEngine::getLastActionReason() const {
    return last_action_reason_;
}

void RuntimeRecoveryEngine::registerRecoveryCallback(std::function<bool(RecoveryAction)> callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    recovery_callbacks_.push_back(callback);
}

void RuntimeRecoveryEngine::registerRecoveryStartCallback(
    std::function<void(RecoveryAction, const std::string&)> callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    start_callbacks_.push_back(callback);
}

void RuntimeRecoveryEngine::registerRecoveryEndCallback(
    std::function<void(RecoveryAction, bool)> callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    end_callbacks_.push_back(callback);
}

void RuntimeRecoveryEngine::beginRecovery(RecoveryAction action, const std::string& reason) {
    in_recovery_ = true;
    last_action_ = action;
    last_action_reason_ = reason;
    
    for (const auto& callback : start_callbacks_) {
        try {
            callback(action, reason);
        } catch (...) {
        }
    }
}

void RuntimeRecoveryEngine::endRecovery(bool success) {
    in_recovery_ = false;
    
    if (success) {
        recovery_count_++;
    }
    
    for (const auto& callback : end_callbacks_) {
        try {
            callback(last_action_, success);
        } catch (...) {
        }
    }
}

RecoveryAction RuntimeRecoveryEngine::mapFailureToAction(FailureType type) {
    switch (type) {
        case FailureType::GPU_OVERRUN:
            return RecoveryAction::RESET_GPU;
            
        case FailureType::AI_CONVERGENCE_COLLAPSE:
            return RecoveryAction::SWITCH_TO_SAFE_POLICY;
            
        case FailureType::FRAME_HASH_MISMATCH:
        case FailureType::REPLAY_DESYNC:
            return RecoveryAction::ROLLBACK_FRAME;
            
        case FailureType::FACTORY_DESYNC:
            return RecoveryAction::ISOLATE_FACTORY;
            
        case FailureType::MEMORY_LEAK_DETECTED:
            return RecoveryAction::REDUCE_FRAME_RATE;
            
        case FailureType::POLICY_CORRUPTION:
            return RecoveryAction::SWITCH_TO_SAFE_POLICY;
            
        case FailureType::NETWORK_TIMEOUT:
            return RecoveryAction::RESTART_SUBSYSTEM;
            
        case FailureType::UNKNOWN:
        default:
            return RecoveryAction::NONE;
    }
}