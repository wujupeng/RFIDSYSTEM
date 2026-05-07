#include "render_graph.h"
#include <algorithm>

RenderGraph::RenderGraph()
    : overlay_pass_(nullptr), frame_stamp_{0, 0, 0}, initialized_(false) {
}

RenderGraph::~RenderGraph() {
    shutdown();
}

void RenderGraph::initialize() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (initialized_) {
        shutdown();
    }
    
    for (auto& pass : passes_) {
        pass->initialize();
        
        if (pass->getType() == PassType::DECISION_OVERLAY) {
            overlay_pass_ = static_cast<DecisionOverlayPass*>(pass.get());
        }
    }
    
    initialized_ = true;
}

void RenderGraph::shutdown() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        return;
    }
    
    for (auto& pass : passes_) {
        pass->shutdown();
    }
    
    passes_.clear();
    overlay_pass_ = nullptr;
    
    initialized_ = false;
}

void RenderGraph::addPass(std::unique_ptr<RenderPass> pass) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    passes_.push_back(std::move(pass));
}

void RenderGraph::execute() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        return;
    }
    
    uint32_t inputTexture = 0;
    
    for (auto& pass : passes_) {
        if (!pass->isEnabled()) {
            continue;
        }
        
        pass->setInput(inputTexture);
        pass->execute();
        
        inputTexture = pass->getOutput();
    }
}

void RenderGraph::setEnabled(PassType type, bool enabled) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    for (auto& pass : passes_) {
        if (pass->getType() == type) {
            pass->setEnabled(enabled);
        }
    }
}

bool RenderGraph::isEnabled(PassType type) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    for (const auto& pass : passes_) {
        if (pass->getType() == type) {
            return pass->isEnabled();
        }
    }
    
    return false;
}

void RenderGraph::setFrameStamp(uint64_t frameId, uint64_t timestamp, uint64_t hash) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    frame_stamp_.frame_id = frameId;
    frame_stamp_.timestamp = timestamp;
    frame_stamp_.hash = hash;
}

void RenderGraph::setOverlayBuffer(uint32_t bufferId, size_t count) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (overlay_pass_) {
        // overlay_pass_->setOverlayBuffer(bufferId, count);
    }
}

void RenderGraph::setCurrentTime(float time) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (overlay_pass_) {
        overlay_pass_->setCurrentTime(time);
    }
}

const std::vector<std::unique_ptr<RenderPass>>& RenderGraph::getPasses() const {
    return passes_;
}