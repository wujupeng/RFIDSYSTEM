#include "decision_overlay_buffer.h"
#include <cstring>

DecisionOverlayBuffer::DecisionOverlayBuffer()
    : mapped_ptr_(nullptr), max_overlays_(100), overlay_count_(0), initialized_(false) {
}

DecisionOverlayBuffer::~DecisionOverlayBuffer() {
    shutdown();
}

void DecisionOverlayBuffer::initialize(size_t maxOverlays) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (initialized_) {
        shutdown();
    }
    
    max_overlays_ = maxOverlays;
    
    auto& pool = GPUBufferPool::instance();
    uint32_t flags = static_cast<uint32_t>(GPUBufferFlags::MAP_PERSISTENT) |
                     static_cast<uint32_t>(GPUBufferFlags::MAP_COHERENT);
    
    gpu_buffer_ = pool.allocateBuffer(
        max_overlays_ * sizeof(GPUDecisionOverlay),
        GPUBufferUsage::STREAM,
        flags
    );
    
    mapped_ptr_ = static_cast<GPUDecisionOverlay*>(gpu_buffer_->map());
    
    initialized_ = true;
}

void DecisionOverlayBuffer::shutdown() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        return;
    }
    
    if (gpu_buffer_) {
        gpu_buffer_->unmap();
        auto& pool = GPUBufferPool::instance();
        pool.releaseBuffer(gpu_buffer_);
        gpu_buffer_.reset();
    }
    
    mapped_ptr_ = nullptr;
    overlay_count_ = 0;
    
    initialized_ = false;
}

void DecisionOverlayBuffer::updateOverlays(const std::vector<GPUDecisionOverlay>& overlays) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_ || !mapped_ptr_) {
        return;
    }
    
    size_t count = std::min(overlays.size(), max_overlays_);
    overlay_count_ = count;
    
    if (count > 0) {
        memcpy(mapped_ptr_, overlays.data(), count * sizeof(GPUDecisionOverlay));
    }
}

uint32_t DecisionOverlayBuffer::getBufferId() const {
    return gpu_buffer_ ? gpu_buffer_->bufferId() : 0;
}

size_t DecisionOverlayBuffer::getOverlayCount() const {
    return overlay_count_;
}