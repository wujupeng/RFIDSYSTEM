#pragma once

#include <cstdint>
#include <mutex>

struct GPUFrameStamp {
    uint64_t frame_id;
    uint64_t timestamp;
    uint64_t spatial_hash;
};

class GPUFrameStampManager {
public:
    static GPUFrameStampManager& instance();
    
    void initialize();
    void shutdown();
    
    void setFrameStamp(uint64_t frameId, uint64_t timestamp, uint64_t hash);
    const GPUFrameStamp& getCurrentFrameStamp() const;
    
    bool isValid() const;
    
    void syncToGPU();
    
private:
    GPUFrameStampManager();
    ~GPUFrameStampManager();
    
    GPUFrameStamp current_stamp_;
    bool valid_;
    
    mutable std::mutex mutex_;
};