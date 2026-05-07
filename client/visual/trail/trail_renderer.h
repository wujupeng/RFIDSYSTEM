#pragma once

#include "trail_types.h"
#include <memory>
#include <mutex>

class TrailRingBuffer;

class TrailRenderer {
public:
    TrailRenderer();
    ~TrailRenderer();
    
    void initialize();
    void shutdown();
    
    void render(const TrailRingBuffer& buffer, float currentTime,
                float viewX, float viewY, float viewWidth, float viewHeight);
    
    void setProjection(float left, float right, float bottom, float top);
    void setDecayFactor(float factor);
    void setMaxAge(float age);
    
private:
    void createShaders();
    void createGeometry();
    
    uint32_t shader_program_;
    uint32_t vao_;
    uint32_t vbo_;
    
    float projection_[16];
    float decay_factor_;
    float max_age_;
    
    bool initialized_;
    std::mutex mutex_;
};