#include "render_pass.h"

RenderPass::RenderPass(const std::string& name, PassType type, PassPriority priority)
    : name_(name), type_(type), priority_(priority), enabled_(true) {
}

RenderPass::~RenderPass() {
}

const std::string& RenderPass::getName() const {
    return name_;
}

RenderPass::PassType RenderPass::getType() const {
    return type_;
}

RenderPass::PassPriority RenderPass::getPriority() const {
    return priority_;
}

void RenderPass::setEnabled(bool enabled) {
    enabled_ = enabled;
}

bool RenderPass::isEnabled() const {
    return enabled_;
}