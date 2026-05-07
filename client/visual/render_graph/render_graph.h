#pragma once

#include "render_pass.h"
#include "decision_overlay_pass.h"
#include <vector>
#include <memory>
#include <mutex>

class RenderGraph {
public:
    RenderGraph();
    ~RenderGraph();
    
    void initialize();
    void shutdown();
    
    void addPass(std::unique_ptr<RenderPass> pass);
    
    void execute();
    
    void setEnabled(PassType type, bool enabled);
    bool isEnabled(PassType type) const;
    
    void setFrameStamp(uint64_t frameId, uint64_t timestamp, uint64_t hash);
    
    void setOverlayBuffer(uint32_t bufferId, size_t count);
    void setCurrentTime(float time);
    
    const std::vector<std::unique_ptr<RenderPass>>& getPasses() const;
    
private:
    std::vector<std::unique_ptr<RenderPass>> passes_;
    std::mutex mutex_;
    
    DecisionOverlayPass* overlay_pass_;
    
    struct {
        uint64_t frame_id;
        uint64_t timestamp;
        uint64_t hash;
    } frame_stamp_;
    
    bool initialized_;
};