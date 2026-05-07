#pragma once

#include <cstdint>
#include <atomic>
#include <mutex>
#include <string>

struct GPUResourceStats {
    size_t vram_used_bytes;
    size_t vram_available_bytes;
    
    uint32_t buffer_count;
    uint32_t texture_count;
    uint32_t fbo_count;
    uint32_t shader_count;
    uint32_t persistent_mapping_count;
    
    double gpu_frame_time_ms;
    double gpu_utilization_percent;
    
    void reset() {
        vram_used_bytes = 0;
        vram_available_bytes = 0;
        buffer_count = 0;
        texture_count = 0;
        fbo_count = 0;
        shader_count = 0;
        persistent_mapping_count = 0;
        gpu_frame_time_ms = 0.0;
        gpu_utilization_percent = 0.0;
    }
};

class GPUResourceTracker {
public:
    static GPUResourceTracker& instance();
    
    void initialize();
    void shutdown();
    
    void trackBufferAllocation(size_t size);
    void trackBufferDeallocation(size_t size);
    
    void trackTextureAllocation(size_t size);
    void trackTextureDeallocation(size_t size);
    
    void incrementFBOCount();
    void decrementFBOCount();
    
    void incrementShaderCount();
    void decrementShaderCount();
    
    void incrementPersistentMappingCount();
    void decrementPersistentMappingCount();
    
    void updateGPUFrameTime(double ms);
    void updateGPUUtilization(double percent);
    
    void updateVRAMStats();
    
    const GPUResourceStats& getStats() const;
    
    bool isVRAMWarning() const;
    bool isVRAMCritical() const;
    
private:
    GPUResourceTracker();
    ~GPUResourceTracker();
    
    mutable std::mutex mutex_;
    
    GPUResourceStats stats_;
    
    std::atomic<size_t> total_buffer_bytes_;
    std::atomic<size_t> total_texture_bytes_;
    
    static constexpr size_t VRAM_WARNING_THRESHOLD_GB = 4;
    static constexpr size_t VRAM_CRITICAL_THRESHOLD_GB = 5;
};