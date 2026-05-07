#pragma once

#include <cstdint>
#include <mutex>
#include <atomic>
#include <string>
#include <functional>

enum class RuntimeMode {
    NORMAL,
    INSPECT,
    DEBUG,
    EMERGENCY
};

enum class ControlCommand {
    STOP_AI,
    RESUME_AI,
    PAUSE_FRAME,
    RESUME_FRAME,
    ROLLBACK_REPLAY,
    ISOLATE_FACTORY,
    RESTORE_FACTORY,
    THROTTLE_GPU,
    RESTORE_GPU,
    FORCE_INSPECT_MODE,
    EXIT_INSPECT_MODE,
    SHUTDOWN,
    RESTART
};

struct ControlState {
    bool ai_enabled;
    bool frame_paused;
    bool gpu_throttled;
    bool inspect_mode;
    bool emergency_mode;
    
    RuntimeMode current_mode;
    
    uint32_t isolated_factory;
    float gpu_throttle_level;
    
    uint64_t last_command_timestamp;
    
    ControlState() 
        : ai_enabled(true), frame_paused(false), gpu_throttled(false),
          inspect_mode(false), emergency_mode(false),
          current_mode(RuntimeMode::NORMAL),
          isolated_factory(0), gpu_throttle_level(1.0f),
          last_command_timestamp(0) {}
};

class RuntimeControlPlane {
public:
    static RuntimeControlPlane& instance();
    
    void initialize();
    void shutdown();
    
    bool executeCommand(ControlCommand command, uint32_t factoryId = 0, float parameter = 0.0f);
    
    const ControlState& getCurrentState() const;
    
    RuntimeMode getCurrentMode() const;
    
    bool isAIEnabled() const;
    bool isFramePaused() const;
    bool isGPUThrottled() const;
    
    void setInspectMode(bool enabled);
    
    void enterEmergencyMode();
    void exitEmergencyMode();
    
    void registerStateChangeCallback(std::function<void(const ControlState&)> callback);
    
    std::string getStatusString() const;
    
private:
    RuntimeControlPlane();
    
    void updateMode();
    void notifyStateChange();
    
    ControlState state_;
    mutable std::mutex mutex_;
    
    std::vector<std::function<void(const ControlState&)>> callbacks_;
    
    static constexpr float MIN_THROTTLE_LEVEL = 0.1f;
};