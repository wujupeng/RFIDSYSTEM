#pragma once

#include <memory>
#include <string>

class RenderPass {
public:
    enum class PassType {
        ASSET,
        TRAIL,
        TRAIL_GLOW,
        HEATMAP,
        HEATMAP_BLUR,
        DECISION_OVERLAY,
        CONGESTION,
        COMPOSITE
    };
    
    enum class PassPriority {
        CRITICAL,
        IMPORTANT,
        OPTIONAL
    };
    
    RenderPass(const std::string& name, PassType type, PassPriority priority = PassPriority::IMPORTANT);
    virtual ~RenderPass();
    
    virtual void initialize() = 0;
    virtual void shutdown() = 0;
    
    virtual void execute() = 0;
    
    virtual void setInput(uint32_t textureId) {}
    virtual uint32_t getOutput() const { return 0; }
    
    const std::string& getName() const;
    PassType getType() const;
    PassPriority getPriority() const;
    
    void setEnabled(bool enabled);
    bool isEnabled() const;
    
protected:
    std::string name_;
    PassType type_;
    PassPriority priority_;
    bool enabled_;
};