#pragma once

#include "render_pass.h"
#include "../gpu/decision_overlay_buffer.h"
#include <memory>
#include <mutex>

class DecisionOverlayPass : public RenderPass {
public:
    DecisionOverlayPass();
    ~DecisionOverlayPass();
    
    void initialize() override;
    void shutdown() override;
    
    void execute() override;
    
    void setInput(uint32_t textureId) override;
    uint32_t getOutput() const override;
    
    void setOverlayBuffer(const DecisionOverlayBuffer* buffer);
    void setCurrentTime(float time);
    
private:
    void createShaders();
    
    uint32_t shader_program_;
    uint32_t vao_;
    uint32_t vbo_;
    
    const DecisionOverlayBuffer* buffer_;
    
    float projection_[16];
    float current_time_;
    
    bool initialized_;
    std::mutex mutex_;
};