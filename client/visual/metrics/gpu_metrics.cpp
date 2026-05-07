#include "gpu_metrics.h"
#include <algorithm>

GPUMetrics::GPUMetrics()
    : fps_(0), dropped_frames_(0), initialized_(false) {
}

GPUMetrics::~GPUMetrics() {
    shutdown();
}

GPUMetrics& GPUMetrics::instance() {
    static GPUMetrics instance;
    return instance;
}

void GPUMetrics::initialize() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (initialized_) {
        return;
    }
    
    history_.reserve(60);
    
    initialized_ = true;
}

void GPUMetrics::shutdown() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        return;
    }
    
    history_.clear();
    
    initialized_ = false;
}

void GPUMetrics::beginFrame(uint64_t frameId, uint64_t timestamp) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    current_metrics_.frame_id = frameId;
    current_metrics_.timestamp = timestamp;
    current_metrics_.cpu_frame_time_ms = 0.0;
    current_metrics_.gpu_frame_time_ms = 0.0;
    current_metrics_.upload_time_ms = 0.0;
    current_metrics_.draw_calls = 0;
    current_metrics_.vram_usage_bytes = 0;
    current_metrics_.fps = fps_.load();
    current_metrics_.dropped_frames = dropped_frames_.load();
}

void GPUMetrics::endFrame() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    history_.push_back(current_metrics_);
    
    if (history_.size() > 60) {
        history_.erase(history_.begin());
    }
}

void GPUMetrics::recordCPUTime(double ms) {
    current_metrics_.cpu_frame_time_ms = ms;
}

void GPUMetrics::recordGPUTime(double ms) {
    current_metrics_.gpu_frame_time_ms = ms;
}

void GPUMetrics::recordUploadTime(double ms) {
    current_metrics_.upload_time_ms = ms;
}

void GPUMetrics::recordDrawCalls(uint32_t count) {
    current_metrics_.draw_calls = count;
}

void GPUMetrics::recordVRAMUsage(size_t bytes) {
    current_metrics_.vram_usage_bytes = bytes;
}

void GPUMetrics::recordAssetCount(size_t count) {
    current_metrics_.asset_count = count;
}

void GPUMetrics::recordTrailCount(size_t count) {
    current_metrics_.trail_count = count;
}

void GPUMetrics::recordDroppedFrame() {
    dropped_frames_.fetch_add(1);
}

const FrameMetrics& GPUMetrics::getCurrentMetrics() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return current_metrics_;
}

std::vector<FrameMetrics> GPUMetrics::getHistory(int count) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (history_.size() <= static_cast<size_t>(count)) {
        return history_;
    }
    
    return std::vector<FrameMetrics>(
        history_.end() - count,
        history_.end()
    );
}

void GPUMetrics::reset() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    history_.clear();
    fps_ = 0;
    dropped_frames_ = 0;
}