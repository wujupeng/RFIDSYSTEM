#pragma once

#include "render_pass.h"
#include <memory>
#include <mutex>

class GPUFrameUploader;

class HeatmapPass : public RenderPass {
public:
    HeatmapPass();
    ~HeatmapPass();
    
    void initialize() override;
    void shutdown() override;
    
    void execute() override;
    
    void setInput(uint32_t textureId) override;
    uint32_t getOutput() const override;
    
    void setUploader(const GPUFrameUploader* uploader);
    
private:
    void createShaders();
    void blurTexture(uint32_t input, uint32_t output);
    
    uint32_t splat_shader_;
    uint32_t blur_shader_;
    uint32_t color_ramp_shader_;
    
    uint32_t splat_fbo_;
    uint32_t blur_fbo_;
    uint32_t output_fbo_;
    
    uint32_t splat_texture_;
    uint32_t blur_texture_;
    uint32_t output_texture_;
    
    const GPUFrameUploader* uploader_;
    
    int texture_size_;
    
    bool initialized_;
    std::mutex mutex_;
};