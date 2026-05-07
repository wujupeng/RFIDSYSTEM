#pragma once

#include "../gpu/gpu_types.h"
#include "gpu_buffer_pool.h"
#include <memory>
#include <mutex>

struct GPUDecisionOverlay {
    float x;
    float y;
    
    float confidence;
    
    uint32_t action;
    uint32_t color;
    
    float pulse;
};

class DecisionOverlayBuffer {
public:
    DecisionOverlayBuffer();
    ~DecisionOverlayBuffer();
    
    void initialize(size_t maxOverlays = 100);
    void shutdown();
    
    void updateOverlays(const std::vector<GPUDecisionOverlay>& overlays);
    
    uint32_t getBufferId() const;
    size_t getOverlayCount() const;
    
private:
    std::mutex mutex_;
    
    std::shared_ptr<GPUBuffer> gpu_buffer_;
    GPUDecisionOverlay* mapped_ptr_;
    
    size_t max_overlays_;
    size_t overlay_count_;
    
    bool initialized_;
};