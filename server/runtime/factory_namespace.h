#pragma once

#include "spatial_frame.h"
#include "ai_analysis_context.h"
#include <string>
#include <vector>
#include <mutex>
#include <memory>
#include <chrono>

class FactoryNamespace {
public:
    using FactoryId = uint32_t;
    
    struct FactoryContext {
        FactoryId id;
        std::string name;
        std::string location;
        bool is_online;
        uint64_t last_sync_timestamp;
        
        std::shared_ptr<SpatialFrame> current_frame;
        std::shared_ptr<AIFrameContext> ai_context;
        
        FactoryContext() 
            : id(0), is_online(false), last_sync_timestamp(0) {}
    };
    
    FactoryNamespace();
    ~FactoryNamespace();
    
    FactoryId registerFactory(const std::string& name, const std::string& location);
    void unregisterFactory(FactoryId id);
    
    bool updateFactoryFrame(FactoryId id, std::shared_ptr<SpatialFrame> frame);
    bool updateFactoryAIContext(FactoryId id, std::shared_ptr<AIFrameContext> context);
    
    FactoryContext* getFactory(FactoryId id);
    std::vector<FactoryContext> getAllFactories() const;
    
    std::vector<FactoryId> getOnlineFactories() const;
    
    size_t getTotalAssetCount() const;
    float getGlobalRiskIndex() const;
    
private:
    std::vector<FactoryContext> factories_;
    std::mutex mutex_;
    FactoryId next_factory_id_;
};