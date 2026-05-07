#include "gpu_frame_stamp.h"

#ifdef _WIN32
#include <windows.h>
#include <GL/gl.h>
#else
#include <GL/gl.h>
#endif

GPUFrameStampManager::GPUFrameStampManager()
    : current_stamp_{0, 0, 0}, valid_(false) {
}

GPUFrameStampManager::~GPUFrameStampManager() {
    shutdown();
}

GPUFrameStampManager& GPUFrameStampManager::instance() {
    static GPUFrameStampManager instance;
    return instance;
}

void GPUFrameStampManager::initialize() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    current_stamp_ = {0, 0, 0};
    valid_ = true;
}

void GPUFrameStampManager::shutdown() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    valid_ = false;
}

void GPUFrameStampManager::setFrameStamp(uint64_t frameId, uint64_t timestamp, uint64_t hash) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    current_stamp_.frame_id = frameId;
    current_stamp_.timestamp = timestamp;
    current_stamp_.spatial_hash = hash;
}

const GPUFrameStamp& GPUFrameStampManager::getCurrentFrameStamp() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return current_stamp_;
}

bool GPUFrameStampManager::isValid() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return valid_;
}

void GPUFrameStampManager::syncToGPU() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!valid_) {
        return;
    }
    
    uint32_t ubo;
    glGenBuffers(1, &ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(GPUFrameStamp), &current_stamp_, GL_STATIC_DRAW);
    
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo);
}