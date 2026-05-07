#pragma once

#include "gpu_types.h"
#include "../gpu/gpu_buffer_pool.h"
#include <memory>
#include <mutex>

class VelocityField {
public:
    VelocityField();
    ~VelocityField();
    
    void initialize(int gridSize = 64);
    void shutdown();
    
    void updateFromAssets(const std::vector<GPUAssetInstance>& assets);
    
    void computeDivergence();
    void computeCurl();
    
    uint32_t getFieldTextureId() const;
    uint32_t getDivergenceTextureId() const;
    uint32_t getCurlTextureId() const;
    
    void getVelocityAt(float x, float y, float& vx, float& vy) const;
    float getDensityAt(float x, float y) const;
    
    int getGridSize() const;
    float getCellSize() const;
    
private:
    void updateTexture();
    
    std::mutex mutex_;
    
    int grid_size_;
    float cell_size_;
    
    std::shared_ptr<GPUBuffer> field_buffer_;
    GPUVelocityField* mapped_field_;
    
    uint32_t field_texture_;
    uint32_t divergence_texture_;
    uint32_t curl_texture_;
    
    bool initialized_;
};