#pragma once

#include "gpu_types.h"
#include <vector>
#include <memory>
#include <mutex>
#include <atomic>

class GPUBuffer {
public:
    GPUBuffer(size_t size, GPUBufferUsage usage, uint32_t flags);
    ~GPUBuffer();
    
    void* map();
    void unmap();
    
    void upload(const void* data, size_t size, size_t offset = 0);
    
    uint32_t bufferId() const;
    size_t size() const;
    
    bool isMapped() const;
    
private:
    uint32_t buffer_id_;
    size_t size_;
    GPUBufferUsage usage_;
    uint32_t flags_;
    
    void* mapped_ptr_;
    bool is_mapped_;
    
    std::mutex mutex_;
};

class GPUBufferPool {
public:
    static GPUBufferPool& instance();
    
    void initialize();
    void shutdown();
    
    std::shared_ptr<GPUBuffer> allocateBuffer(size_t size, 
                                              GPUBufferUsage usage = GPUBufferUsage::STREAM,
                                              uint32_t flags = 0);
    
    void releaseBuffer(std::shared_ptr<GPUBuffer> buffer);
    
    void flushAll();
    
    size_t totalAllocated() const;
    size_t totalUsed() const;
    
private:
    GPUBufferPool();
    ~GPUBufferPool();
    
    std::vector<std::shared_ptr<GPUBuffer>> buffers_;
    std::mutex mutex_;
    
    std::atomic<size_t> total_allocated_;
    std::atomic<size_t> total_used_;
    
    bool initialized_;
};