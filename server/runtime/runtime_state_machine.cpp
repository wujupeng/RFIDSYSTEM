#include "runtime_state_machine.h"

const std::string RuntimeStateMachine::STATE_NAMES[8] = {
    "INIT",
    "RUNNING",
    "DEGRADED",
    "INSPECT",
    "PAUSED",
    "RECOVERING",
    "EMERGENCY_STOP",
    "SHUTDOWN"
};

RuntimeStateMachine::RuntimeStateMachine()
    : state_(RuntimeState::INIT),
      state_name_("INIT"),
      last_reason_(""),
      state_change_count_(0) {
}

RuntimeStateMachine& RuntimeStateMachine::instance() {
    static RuntimeStateMachine instance;
    return instance;
}

void RuntimeStateMachine::initialize() {
    std::lock_guard<std::mutex> lock(mutex_);
    state_ = RuntimeState::INIT;
    state_name_ = STATE_NAMES[0];
    last_reason_ = "System initialized";
    state_change_count_ = 0;
}

void RuntimeStateMachine::shutdown() {
    std::lock_guard<std::mutex> lock(mutex_);
    state_ = RuntimeState::SHUTDOWN;
    state_name_ = STATE_NAMES[7];
    last_reason_ = "System shutdown";
}

RuntimeState RuntimeStateMachine::current() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return state_;
}

bool RuntimeStateMachine::transition(RuntimeState target, const std::string& reason) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    RuntimeState from = state_;
    
    if (!isValidTransition(from, target)) {
        return false;
    }
    
    state_ = target;
    state_name_ = STATE_NAMES[static_cast<int>(target)];
    last_reason_ = reason;
    state_change_count_++;
    
    for (const auto& callback : callbacks_) {
        try {
            callback(from, target);
        } catch (...) {
        }
    }
    
    return true;
}

bool RuntimeStateMachine::isHealthy() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return state_ == RuntimeState::RUNNING || state_ == RuntimeState::DEGRADED;
}

bool RuntimeStateMachine::canRunAI() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    switch (state_) {
        case RuntimeState::RUNNING:
        case RuntimeState::DEGRADED:
            return true;
        case RuntimeState::INSPECT:
        case RuntimeState::PAUSED:
        case RuntimeState::RECOVERING:
            return false;
        case RuntimeState::EMERGENCY_STOP:
        case RuntimeState::SHUTDOWN:
        case RuntimeState::INIT:
            return false;
    }
    return false;
}

bool RuntimeStateMachine::canRenderGPU() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    switch (state_) {
        case RuntimeState::RUNNING:
        case RuntimeState::DEGRADED:
        case RuntimeState::INSPECT:
            return true;
        case RuntimeState::PAUSED:
            return false;
        case RuntimeState::RECOVERING:
            return false;
        case RuntimeState::EMERGENCY_STOP:
        case RuntimeState::SHUTDOWN:
        case RuntimeState::INIT:
            return false;
    }
    return false;
}

bool RuntimeStateMachine::canScheduleFrame() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    switch (state_) {
        case RuntimeState::RUNNING:
        case RuntimeState::DEGRADED:
        case RuntimeState::RECOVERING:
            return true;
        case RuntimeState::INSPECT:
            return true;
        case RuntimeState::PAUSED:
            return false;
        case RuntimeState::EMERGENCY_STOP:
        case RuntimeState::SHUTDOWN:
        case RuntimeState::INIT:
            return false;
    }
    return false;
}

bool RuntimeStateMachine::canUpdatePolicy() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    switch (state_) {
        case RuntimeState::RUNNING:
            return true;
        case RuntimeState::DEGRADED:
            return true;
        case RuntimeState::RECOVERING:
            return false;
        case RuntimeState::INSPECT:
            return false;
        case RuntimeState::PAUSED:
            return false;
        case RuntimeState::EMERGENCY_STOP:
        case RuntimeState::SHUTDOWN:
        case RuntimeState::INIT:
            return false;
    }
    return false;
}

bool RuntimeStateMachine::canWriteDecision() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    switch (state_) {
        case RuntimeState::RUNNING:
        case RuntimeState::DEGRADED:
            return true;
        case RuntimeState::INSPECT:
            return false;
        case RuntimeState::PAUSED:
            return false;
        case RuntimeState::RECOVERING:
            return false;
        case RuntimeState::EMERGENCY_STOP:
        case RuntimeState::SHUTDOWN:
        case RuntimeState::INIT:
            return false;
    }
    return false;
}

const std::string& RuntimeStateMachine::getCurrentStateName() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return state_name_;
}

const std::string& RuntimeStateMachine::getLastTransitionReason() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return last_reason_;
}

void RuntimeStateMachine::registerStateChangeCallback(std::function<void(RuntimeState, RuntimeState)> callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    callbacks_.push_back(callback);
}

size_t RuntimeStateMachine::getStateChangeCount() const {
    return state_change_count_;
}

bool RuntimeStateMachine::isValidTransition(RuntimeState from, RuntimeState to) const {
    switch (from) {
        case RuntimeState::INIT:
            return to == RuntimeState::RUNNING || to == RuntimeState::SHUTDOWN;
            
        case RuntimeState::RUNNING:
            return to == RuntimeState::DEGRADED || 
                   to == RuntimeState::INSPECT || 
                   to == RuntimeState::PAUSED || 
                   to == RuntimeState::EMERGENCY_STOP ||
                   to == RuntimeState::SHUTDOWN;
            
        case RuntimeState::DEGRADED:
            return to == RuntimeState::RUNNING || 
                   to == RuntimeState::INSPECT || 
                   to == RuntimeState::PAUSED || 
                   to == RuntimeState::EMERGENCY_STOP ||
                   to == RuntimeState::SHUTDOWN;
            
        case RuntimeState::INSPECT:
            return to == RuntimeState::RUNNING || 
                   to == RuntimeState::DEGRADED || 
                   to == RuntimeState::PAUSED || 
                   to == RuntimeState::EMERGENCY_STOP ||
                   to == RuntimeState::SHUTDOWN;
            
        case RuntimeState::PAUSED:
            return to == RuntimeState::RUNNING || 
                   to == RuntimeState::DEGRADED || 
                   to == RuntimeState::RECOVERING ||
                   to == RuntimeState::EMERGENCY_STOP ||
                   to == RuntimeState::SHUTDOWN;
            
        case RuntimeState::RECOVERING:
            return to == RuntimeState::RUNNING || 
                   to == RuntimeState::DEGRADED || 
                   to == RuntimeState::EMERGENCY_STOP ||
                   to == RuntimeState::SHUTDOWN;
            
        case RuntimeState::EMERGENCY_STOP:
            return to == RuntimeState::RECOVERING ||
                   to == RuntimeState::SHUTDOWN;
            
        case RuntimeState::SHUTDOWN:
            return false;
    }
    
    return false;
}