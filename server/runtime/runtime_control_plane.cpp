#include "runtime_control_plane.h"
#include <sstream>

RuntimeControlPlane::RuntimeControlPlane() {
}

RuntimeControlPlane& RuntimeControlPlane::instance() {
    static RuntimeControlPlane instance;
    return instance;
}

void RuntimeControlPlane::initialize() {
    state_ = ControlState();
}

void RuntimeControlPlane::shutdown() {
    std::lock_guard<std::mutex> lock(mutex_);
    callbacks_.clear();
}

bool RuntimeControlPlane::executeCommand(ControlCommand command, uint32_t factoryId, float parameter) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    bool success = true;
    
    switch (command) {
        case ControlCommand::STOP_AI:
            state_.ai_enabled = false;
            break;
            
        case ControlCommand::RESUME_AI:
            state_.ai_enabled = true;
            break;
            
        case ControlCommand::PAUSE_FRAME:
            state_.frame_paused = true;
            break;
            
        case ControlCommand::RESUME_FRAME:
            state_.frame_paused = false;
            break;
            
        case ControlCommand::ROLLBACK_REPLAY:
            break;
            
        case ControlCommand::ISOLATE_FACTORY:
            state_.isolated_factory = factoryId;
            break;
            
        case ControlCommand::RESTORE_FACTORY:
            if (state_.isolated_factory == factoryId) {
                state_.isolated_factory = 0;
            }
            break;
            
        case ControlCommand::THROTTLE_GPU:
            state_.gpu_throttled = true;
            state_.gpu_throttle_level = std::max(MIN_THROTTLE_LEVEL, parameter);
            break;
            
        case ControlCommand::RESTORE_GPU:
            state_.gpu_throttled = false;
            state_.gpu_throttle_level = 1.0f;
            break;
            
        case ControlCommand::FORCE_INSPECT_MODE:
            state_.inspect_mode = true;
            break;
            
        case ControlCommand::EXIT_INSPECT_MODE:
            state_.inspect_mode = false;
            break;
            
        case ControlCommand::SHUTDOWN:
            break;
            
        case ControlCommand::RESTART:
            break;
            
        default:
            success = false;
            break;
    }
    
    state_.last_command_timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    
    updateMode();
    notifyStateChange();
    
    return success;
}

const ControlState& RuntimeControlPlane::getCurrentState() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return state_;
}

RuntimeMode RuntimeControlPlane::getCurrentMode() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return state_.current_mode;
}

bool RuntimeControlPlane::isAIEnabled() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return state_.ai_enabled;
}

bool RuntimeControlPlane::isFramePaused() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return state_.frame_paused;
}

bool RuntimeControlPlane::isGPUThrottled() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return state_.gpu_throttled;
}

void RuntimeControlPlane::setInspectMode(bool enabled) {
    std::lock_guard<std::mutex> lock(mutex_);
    state_.inspect_mode = enabled;
    updateMode();
    notifyStateChange();
}

void RuntimeControlPlane::enterEmergencyMode() {
    std::lock_guard<std::mutex> lock(mutex_);
    state_.emergency_mode = true;
    state_.ai_enabled = false;
    updateMode();
    notifyStateChange();
}

void RuntimeControlPlane::exitEmergencyMode() {
    std::lock_guard<std::mutex> lock(mutex_);
    state_.emergency_mode = false;
    state_.ai_enabled = true;
    updateMode();
    notifyStateChange();
}

void RuntimeControlPlane::registerStateChangeCallback(std::function<void(const ControlState&)> callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    callbacks_.push_back(callback);
}

std::string RuntimeControlPlane::getStatusString() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::stringstream ss;
    
    switch (state_.current_mode) {
        case RuntimeMode::NORMAL:
            ss << "NORMAL";
            break;
        case RuntimeMode::INSPECT:
            ss << "INSPECT";
            break;
        case RuntimeMode::DEBUG:
            ss << "DEBUG";
            break;
        case RuntimeMode::EMERGENCY:
            ss << "EMERGENCY";
            break;
    }
    
    ss << " | AI:" << (state_.ai_enabled ? "ON" : "OFF");
    ss << " | Frame:" << (state_.frame_paused ? "PAUSED" : "RUNNING");
    ss << " | GPU:" << (state_.gpu_throttled ? "THROTTLED" : "NORMAL");
    
    return ss.str();
}

void RuntimeControlPlane::updateMode() {
    if (state_.emergency_mode) {
        state_.current_mode = RuntimeMode::EMERGENCY;
    } else if (state_.inspect_mode) {
        state_.current_mode = RuntimeMode::INSPECT;
    } else {
        state_.current_mode = RuntimeMode::NORMAL;
    }
}

void RuntimeControlPlane::notifyStateChange() {
    for (const auto& callback : callbacks_) {
        try {
            callback(state_);
        } catch (...) {
        }
    }
}