#include "gpu_watchdog.h"
#include <thread>

GPUWatchdog::GPUWatchdog()
    : running_(false), watchdog_triggered_(false),
      reset_count_(0), max_frame_time_ms_(0.0) {
}

GPUWatchdog::~GPUWatchdog() {
    shutdown();
}

GPUWatchdog& GPUWatchdog::instance() {
    static GPUWatchdog instance;
    return instance;
}

void GPUWatchdog::initialize(ResetCallback callback) {
    reset_callback_ = callback;
    running_ = true;
}

void GPUWatchdog::shutdown() {
    running_ = false;
}

void GPUWatchdog::startFrame() {
    frame_start_ = std::chrono::high_resolution_clock::now();
    watchdog_triggered_ = false;
}

void GPUWatchdog::endFrame() {
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration<double, std::milli>(end - frame_start_);
    double frame_time_ms = duration.count();
    
    if (frame_time_ms > max_frame_time_ms_) {
        max_frame_time_ms_ = frame_time_ms;
    }
    
    checkTimeout();
}

void GPUWatchdog::registerPass(const std::string& passName) {
    std::lock_guard<std::mutex> lock(mutex_);
}

void GPUWatchdog::unregisterPass(const std::string& passName) {
    std::lock_guard<std::mutex> lock(mutex_);
}

void GPUWatchdog::resetPass(const std::string& passName) {
    if (reset_callback_) {
        reset_callback_();
    }
    reset_count_++;
    watchdog_triggered_ = false;
}

bool GPUWatchdog::isWatchdogTriggered() const {
    return watchdog_triggered_;
}

uint32_t GPUWatchdog::getResetCount() const {
    return reset_count_;
}

double GPUWatchdog::getMaxFrameTime() const {
    return max_frame_time_ms_;
}

void GPUWatchdog::checkTimeout() {
    auto now = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration<double, std::milli>(now - frame_start_);
    
    if (duration.count() > FRAME_TIMEOUT_MS) {
        watchdog_triggered_ = true;
        
        if (reset_callback_) {
            reset_callback_();
        }
        
        reset_count_++;
    }
}