#pragma once

#include "trail_types.h"
#include "../gpu/gpu_buffer_pool.h"
#include <memory>
#include <mutex>
#include <atomic>
#include <unordered_map>

class TrailRingBuffer {
public:
    TrailRingBuffer();
    ~TrailRingBuffer();
    
    void initialize(const TrailConfig& config);
    void shutdown();
    
    void addPoint(uint32_t assetId, float x, float y, float timestamp, 
                  float intensity = 1.0f, uint32_t color = 0xFFFFFFFF);
    
    void addPoints(uint32_t assetId, const std::vector<GPUTrailPoint>& points);
    
    void updateAssetVelocity(uint32_t assetId, float vx, float vy);
    
    void cleanupOldPoints(float currentTime);
    
    uint32_t getBufferId() const;
    size_t getPointCount() const;
    
    const TrailConfig& getConfig() const;
    TrailStats getStats() const;
    
    void flush();
    
private:
    void growIfNeeded();
    
    std::mutex mutex_;
    
    TrailConfig config_;
    
    std::shared_ptr<GPUBuffer> gpu_buffer_;
    GPUTrailPoint* mapped_ptr_;
    
    std::atomic<size_t> write_head_;
    std::atomic<size_t> read_head_;
    
    std::unordered_map<uint32_t, size_t> asset_point_counts_;
    std::unordered_map<uint32_t, std::pair<float, float>> asset_velocities_;
    
    bool initialized_;
};