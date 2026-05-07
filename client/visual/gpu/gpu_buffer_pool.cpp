#include "gpu_buffer_pool.h"
#include <stdexcept>

#ifdef _WIN32
#include <windows.h>
#include <GL/gl.h>
#else
#include <GL/gl.h>
#endif

GPUBuffer::GPUBuffer(size_t size, GPUBufferUsage usage, uint32_t flags)
    : size_(size), usage_(usage), flags_(flags), mapped_ptr_(nullptr), is_mapped_(false) {
    
    uint32_t glUsage = GL_STATIC_DRAW;
    switch (usage) {
        case GPUBufferUsage::STATIC:
            glUsage = GL_STATIC_DRAW;
            break;
        case GPUBufferUsage::DYNAMIC:
            glUsage = GL_DYNAMIC_DRAW;
            break;
        case GPUBufferUsage::STREAM:
            glUsage = GL_STREAM_DRAW;
            break;
    }
    
    glGenBuffers(1, &buffer_id_);
    glBindBuffer(GL_ARRAY_BUFFER, buffer_id_);
    glBufferData(GL_ARRAY_BUFFER, size, nullptr, glUsage);
    
    if (flags & static_cast<uint32_t>(GPUBufferFlags::MAP_PERSISTENT)) {
        uint32_t mapFlags = GL_MAP_WRITE_BIT;
        if (flags & static_cast<uint32_t>(GPUBufferFlags::MAP_COHERENT)) {
            mapFlags |= GL_MAP_COHERENT_BIT;
        }
        if (flags & static_cast<uint32_t>(GPUBufferFlags::MAP_WRITE_INVALIDATE)) {
            mapFlags |= GL_MAP_INVALIDATE_BUFFER_BIT;
        }
        
        mapped_ptr_ = glMapBufferRange(GL_ARRAY_BUFFER, 0, size, mapFlags);
        is_mapped_ = (mapped_ptr_ != nullptr);
    }
}

GPUBuffer::~GPUBuffer() {
    if (is_mapped_ && mapped_ptr_) {
        unmap();
    }
    
    if (buffer_id_ != 0) {
        glDeleteBuffers(1, &buffer_id_);
    }
}

void* GPUBuffer::map() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (is_mapped_) {
        return mapped_ptr_;
    }
    
    uint32_t mapFlags = GL_MAP_WRITE_BIT;
    if (flags_ & static_cast<uint32_t>(GPUBufferFlags::MAP_WRITE_INVALIDATE)) {
        mapFlags |= GL_MAP_INVALIDATE_BUFFER_BIT;
    }
    
    glBindBuffer(GL_ARRAY_BUFFER, buffer_id_);
    mapped_ptr_ = glMapBufferRange(GL_ARRAY_BUFFER, 0, size_, mapFlags);
    is_mapped_ = (mapped_ptr_ != nullptr);
    
    return mapped_ptr_;
}

void GPUBuffer::unmap() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!is_mapped_ || !mapped_ptr_) {
        return;
    }
    
    glBindBuffer(GL_ARRAY_BUFFER, buffer_id_);
    glUnmapBuffer(GL_ARRAY_BUFFER);
    
    mapped_ptr_ = nullptr;
    is_mapped_ = false;
}

void GPUBuffer::upload(const void* data, size_t size, size_t offset) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (offset + size > size_) {
        throw std::runtime_error("Upload exceeds buffer size");
    }
    
    glBindBuffer(GL_ARRAY_BUFFER, buffer_id_);
    
    if (is_mapped_ && mapped_ptr_) {
        memcpy(static_cast<uint8_t*>(mapped_ptr_) + offset, data, size);
    } else {
        glBufferSubData(GL_ARRAY_BUFFER, offset, size, data);
    }
}

uint32_t GPUBuffer::bufferId() const {
    return buffer_id_;
}

size_t GPUBuffer::size() const {
    return size_;
}

bool GPUBuffer::isMapped() const {
    return is_mapped_;
}

GPUBufferPool::GPUBufferPool() 
    : total_allocated_(0), total_used_(0), initialized_(false) {
}

GPUBufferPool::~GPUBufferPool() {
    shutdown();
}

GPUBufferPool& GPUBufferPool::instance() {
    static GPUBufferPool instance;
    return instance;
}

void GPUBufferPool::initialize() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (initialized_) {
        return;
    }
    
    buffers_.clear();
    total_allocated_ = 0;
    total_used_ = 0;
    
    initialized_ = true;
}

void GPUBufferPool::shutdown() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    buffers_.clear();
    total_allocated_ = 0;
    total_used_ = 0;
    
    initialized_ = false;
}

std::shared_ptr<GPUBuffer> GPUBufferPool::allocateBuffer(size_t size, 
                                                         GPUBufferUsage usage,
                                                         uint32_t flags) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto buffer = std::make_shared<GPUBuffer>(size, usage, flags);
    buffers_.push_back(buffer);
    
    total_allocated_ += size;
    total_used_ += size;
    
    return buffer;
}

void GPUBufferPool::releaseBuffer(std::shared_ptr<GPUBuffer> buffer) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = std::find(buffers_.begin(), buffers_.end(), buffer);
    if (it != buffers_.end()) {
        total_used_ -= buffer->size();
        buffers_.erase(it);
    }
}

void GPUBufferPool::flushAll() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    for (auto& buffer : buffers_) {
        if (buffer->isMapped()) {
            buffer->unmap();
        }
    }
}

size_t GPUBufferPool::totalAllocated() const {
    return total_allocated_;
}

size_t GPUBufferPool::totalUsed() const {
    return total_used_;
}