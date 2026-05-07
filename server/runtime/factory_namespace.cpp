#include "factory_namespace.h"

FactoryNamespace::FactoryNamespace()
    : next_factory_id_(1) {
}

FactoryNamespace::~FactoryNamespace() {
}

FactoryNamespace::FactoryId FactoryNamespace::registerFactory(const std::string& name, const std::string& location) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    FactoryContext context;
    context.id = next_factory_id_++;
    context.name = name;
    context.location = location;
    context.is_online = true;
    context.last_sync_timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    
    factories_.push_back(context);
    
    return context.id;
}

void FactoryNamespace::unregisterFactory(FactoryId id) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = std::find_if(factories_.begin(), factories_.end(),
                          [id](const FactoryContext& f) { return f.id == id; });
    
    if (it != factories_.end()) {
        it->is_online = false;
    }
}

bool FactoryNamespace::updateFactoryFrame(FactoryId id, std::shared_ptr<SpatialFrame> frame) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = std::find_if(factories_.begin(), factories_.end(),
                          [id](const FactoryContext& f) { return f.id == id; });
    
    if (it != factories_.end()) {
        it->current_frame = frame;
        it->last_sync_timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        return true;
    }
    
    return false;
}

bool FactoryNamespace::updateFactoryAIContext(FactoryId id, std::shared_ptr<AIFrameContext> context) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = std::find_if(factories_.begin(), factories_.end(),
                          [id](const FactoryContext& f) { return f.id == id; });
    
    if (it != factories_.end()) {
        it->ai_context = context;
        it->last_sync_timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        return true;
    }
    
    return false;
}

FactoryNamespace::FactoryContext* FactoryNamespace::getFactory(FactoryId id) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = std::find_if(factories_.begin(), factories_.end(),
                          [id](const FactoryContext& f) { return f.id == id; });
    
    if (it != factories_.end()) {
        return &(*it);
    }
    
    return nullptr;
}

std::vector<FactoryNamespace::FactoryContext> FactoryNamespace::getAllFactories() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return factories_;
}

std::vector<FactoryNamespace::FactoryId> FactoryNamespace::getOnlineFactories() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::vector<FactoryId> online;
    for (const auto& factory : factories_) {
        if (factory.is_online) {
            online.push_back(factory.id);
        }
    }
    
    return online;
}

size_t FactoryNamespace::getTotalAssetCount() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    size_t total = 0;
    for (const auto& factory : factories_) {
        if (factory.current_frame) {
            total += factory.current_frame->assets.size();
        }
    }
    
    return total;
}

float FactoryNamespace::getGlobalRiskIndex() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    float sumRisk = 0.0f;
    size_t count = 0;
    
    for (const auto& factory : factories_) {
        if (factory.ai_context) {
            sumRisk += factory.ai_context->global_risk_index;
            count++;
        }
    }
    
    return count > 0 ? sumRisk / count : 0.0f;
}