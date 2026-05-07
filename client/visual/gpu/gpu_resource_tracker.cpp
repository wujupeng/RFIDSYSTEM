#include "gpu_resource_tracker.h"
#include <cstring>

GPUResourceTracker::GPUResourceTracker()
    : total_buffer_bytes_(0), total_texture_bytes_(0) {
    stats_.reset();
}

GPUResourceTracker::~GPUResourceTracker() {
    shutdown();
}

GPUResourceTracker& GPUResourceTracker::instance() {
    static GPUResourceTracker instance;
    return instance;
}

void GPUResourceTracker::initialize() {
    stats_.reset();
    total_buffer_bytes_ = 0;
    total_texture_bytes_ = 0;
}

void GPUResourceTracker::shutdown() {
    stats_.reset();
}

void GPUResourceTracker::trackBufferAllocation(size_t size) {
    total_buffer_bytes_ += size;
}

void GPUResourceTracker::trackBufferDeallocation(size_t size) {
    total_buffer_bytes_ -= size;
}

void GPUResourceTracker::trackTextureAllocation(size_t size) {
    total_texture_bytes_ += size;
}

void GPUResourceTracker::trackTextureDeallocation(size_t size) {
    total_texture_bytes_ -= size;
}

void GPUResourceTracker::incrementFBOCount() {
    std::lock_guard<std::mutex> lock(mutex_);
    stats_.fbo_count++;
}

void GPUResourceTracker::decrementFBOCount() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (stats_.fbo_count > 0) {
        stats_.fbo_count--;
    }
}

void GPUResourceTracker::incrementShaderCount() {
    std::lock_guard<std::mutex> lock(mutex_);
    stats_.shader_count++;
}

void GPUResourceTracker::decrementShaderCount() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (stats_.shader_count > 0) {
        stats_.shader_count--;
    }
}

void GPUResourceTracker::incrementPersistentMappingCount() {
    std::lock_guard<std::mutex> lock(mutex_);
    stats_.persistent_mapping_count++;
}

void GPUResourceTracker::decrementPersistentMappingCount() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (stats_.persistent_mapping_count > 0) {
        stats_.persistent_mapping_count--;
    }
}

void GPUResourceTracker::updateGPUFrameTime(double ms) {
    std::lock_guard<std::mutex> lock(mutex_);
    stats_.gpu_frame_time_ms = ms;
}

void GPUResourceTracker::updateGPUUtilization(double percent) {
    std::lock_guard<std::mutex> lock(mutex_);
    stats_.gpu_utilization_percent = percent;
}

void GPUResourceTracker::updateVRAMStats() {
    std::lock_guard<std::mutex> lock(mutex_);
    stats_.vram_used_bytes = total_buffer_bytes_ + total_texture_bytes_;
    
    stats_.buffer_count = static_cast<uint32_t>(total_buffer_bytes_ / 1024 / 1024);
}

const GPUResourceStats& GPUResourceTracker::getStats() const {
    return stats_;
}

bool GPUResourceTracker::isVRAMWarning() const {
    return stats_.vram_used_bytes > VRAM_WARNING_THRESHOLD_GB * 1024 * 1024 * 1024;
}

bool GPUResourceTracker::isVRAMCritical() const {
    return stats_.vram_used_bytes > VRAM_CRITICAL_THRESHOLD_GB * 1024 * 1024 * 1024;
}