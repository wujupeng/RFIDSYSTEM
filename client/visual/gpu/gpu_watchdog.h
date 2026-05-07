#pragma once

#include <atomic>
#include <mutex>
#include <chrono>
#include <functional>

class GPUWatchdog {
public:
    using ResetCallback = std::function<void()>;
    
    static GPUWatchdog& instance();
    
    void initialize(ResetCallback callback);
    void shutdown();
    
    void startFrame();
    void endFrame();
    
    void registerPass(const std::string& passName);
    void unregisterPass(const std::string& passName);
    
    void resetPass(const std::string& passName);
    
    bool isWatchdogTriggered() const;
    uint32_t getResetCount() const;
    double getMaxFrameTime() const;
    
private:
    GPUWatchdog();
    ~GPUWatchdog();
    
    void checkTimeout();
    
    ResetCallback reset_callback_;
    
    std::atomic<bool> running_;
    std::atomic<bool> watchdog_triggered_;
    
    std::atomic<uint32_t> reset_count_;
    std::atomic<double> max_frame_time_ms_;
    
    std::chrono::high_resolution_clock::time_point frame_start_;
    
    std::mutex mutex_;
    
    static constexpr double FRAME_TIMEOUT_MS = 500.0;
    static constexpr double WARNING_TIMEOUT_MS = 200.0;
};