#pragma once

#include "runtime_failure_bus.h"
#include <string>
#include <mutex>
#include <atomic>
#include <vector>
#include <functional>

enum class RecoveryAction {
    NONE,
    RESTART_AI,
    RESET_GPU,
    ROLLBACK_FRAME,
    ISOLATE_FACTORY,
    SWITCH_TO_SAFE_POLICY,
    REDUCE_FRAME_RATE,
    DISABLE_OVERLAY,
    RESTART_SUBSYSTEM,
    EMERGENCY_STOP
};

class RuntimeRecoveryEngine {
public:
    static RuntimeRecoveryEngine& instance();
    
    void initialize();
    void shutdown();
    
    RecoveryAction decide(const FailureEvent& event);
    
    bool execute(RecoveryAction action);
    
    bool inRecovery() const;
    
    size_t getRecoveryCount() const;
    
    RecoveryAction getLastAction() const;
    
    const std::string& getLastActionReason() const;
    
    void registerRecoveryCallback(std::function<bool(RecoveryAction)> callback);
    
    void registerRecoveryStartCallback(std::function<void(RecoveryAction, const std::string&)> callback);
    void registerRecoveryEndCallback(std::function<void(RecoveryAction, bool)> callback);
    
    void beginRecovery(RecoveryAction action, const std::string& reason);
    void endRecovery(bool success);
    
private:
    RuntimeRecoveryEngine();
    
    RecoveryAction mapFailureToAction(FailureType type);
    
    std::atomic<bool> in_recovery_;
    std::atomic<size_t> recovery_count_;
    
    RecoveryAction last_action_;
    std::string last_action_reason_;
    
    mutable std::mutex mutex_;
    
    std::vector<std::function<bool(RecoveryAction)>> recovery_callbacks_;
    std::vector<std::function<void(RecoveryAction, const std::string&)>> start_callbacks_;
    std::vector<std::function<void(RecoveryAction, bool)>> end_callbacks_;
};