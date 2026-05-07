#pragma once

#include <cstdint>
#include <mutex>
#include <atomic>
#include <vector>

struct FrameMetrics {
    uint64_t frame_id;
    uint64_t timestamp;
    
    double cpu_frame_time_ms;
    double gpu_frame_time_ms;
    double upload_time_ms;
    
    uint32_t draw_calls;
    size_t vram_usage_bytes;
    
    uint32_t fps;
    uint32_t dropped_frames;
    
    size_t asset_count;
    size_t trail_count;
};

class GPUMetrics {
public:
    static GPUMetrics& instance();
    
    void initialize();
    void shutdown();
    
    void beginFrame(uint64_t frameId, uint64_t timestamp);
    void endFrame();
    
    void recordCPUTime(double ms);
    void recordGPUTime(double ms);
    void recordUploadTime(double ms);
    
    void recordDrawCalls(uint32_t count);
    void recordVRAMUsage(size_t bytes);
    
    void recordAssetCount(size_t count);
    void recordTrailCount(size_t count);
    
    void recordDroppedFrame();
    
    const FrameMetrics& getCurrentMetrics() const;
    std::vector<FrameMetrics> getHistory(int count) const;
    
    void reset();
    
private:
    GPUMetrics();
    ~GPUMetrics();
    
    FrameMetrics current_metrics_;
    std::vector<FrameMetrics> history_;
    
    std::atomic<uint32_t> fps_;
    std::atomic<uint32_t> dropped_frames_;
    
    mutable std::mutex mutex_;
    
    bool initialized_;
};