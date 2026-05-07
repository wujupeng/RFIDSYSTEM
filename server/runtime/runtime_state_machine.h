#pragma once

#include <string>
#include <mutex>
#include <atomic>
#include <vector>
#include <functional>

enum class RuntimeState {
    INIT,
    RUNNING,
    DEGRADED,
    INSPECT,
    PAUSED,
    RECOVERING,
    EMERGENCY_STOP,
    SHUTDOWN
};

class RuntimeStateMachine {
public:
    static RuntimeStateMachine& instance();
    
    void initialize();
    void shutdown();
    
    RuntimeState current() const;
    
    bool transition(RuntimeState target, const std::string& reason);
    
    bool isHealthy() const;
    
    bool canRunAI() const;
    bool canRenderGPU() const;
    bool canScheduleFrame() const;
    bool canUpdatePolicy() const;
    bool canWriteDecision() const;
    
    const std::string& getCurrentStateName() const;
    const std::string& getLastTransitionReason() const;
    
    void registerStateChangeCallback(std::function<void(RuntimeState, RuntimeState)> callback);
    
    size_t getStateChangeCount() const;
    
private:
    RuntimeStateMachine();
    
    bool isValidTransition(RuntimeState from, RuntimeState to) const;
    
    RuntimeState state_;
    std::string state_name_;
    std::string last_reason_;
    
    mutable std::mutex mutex_;
    
    std::vector<std::function<void(RuntimeState, RuntimeState)>> callbacks_;
    std::atomic<size_t> state_change_count_;
    
    static const std::string STATE_NAMES[8];
};