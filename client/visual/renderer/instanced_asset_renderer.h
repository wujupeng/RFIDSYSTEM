#pragma once

#include "../gpu/gpu_types.h"
#include <memory>
#include <mutex>

class GPUFrameUploader;

class InstancedAssetRenderer {
public:
    InstancedAssetRenderer();
    ~InstancedAssetRenderer();
    
    void initialize();
    void shutdown();
    
    void render(const GPUFrameUploader& uploader, 
                float viewX, float viewY, 
                float viewWidth, float viewHeight);
    
    void setPointSize(float size);
    float getPointSize() const;
    
    void setGlowEnabled(bool enabled);
    bool isGlowEnabled() const;
    
    void setProjection(float left, float right, float bottom, float top);
    
private:
    void createShaders();
    void createGeometry();
    void setupVertexAttributes();
    
    uint32_t shader_program_;
    uint32_t vao_;
    uint32_t vbo_;
    
    float point_size_;
    bool glow_enabled_;
    
    float projection_[16];
    
    bool initialized_;
    std::mutex mutex_;
};