#pragma once

#include "factory_namespace.h"
#include "ai_analysis_context.h"
#include <mutex>
#include <atomic>
#include <vector>
#include <functional>

enum class FactoryEventType {
    RISK_THRESHOLD_EXCEEDED,
    ANOMALY_DETECTED,
    FACTORY_OFFLINE,
    FACTORY_ONLINE,
    SYNC_REQUEST,
    SYNC_COMPLETE
};

struct FactoryEvent {
    FactoryEventType type;
    uint32_t factory_id;
    uint64_t timestamp;
    float risk_value;
    std::string message;
};

class FactoryFederation {
public:
    static FactoryFederation& instance();
    
    void initialize();
    void shutdown();
    
    void registerFactory(uint32_t factoryId, const std::string& name, const std::string& location);
    void unregisterFactory(uint32_t factoryId);
    
    void updateFactoryRisk(uint32_t factoryId, float risk, const AIFrameContext& context);
    
    float getGlobalRisk() const;
    
    std::vector<uint32_t> getHighRiskFactories(float threshold = 0.7f) const;
    
    void propagateAnomaly(uint32_t sourceFactoryId);
    
    void syncFactories();
    
    void registerEventCallback(std::function<void(const FactoryEvent&)> callback);
    
    const std::vector<FactoryEvent>& getRecentEvents(size_t count = 50) const;
    
    size_t getAnomalyPropagationCount() const;
    
private:
    FactoryFederation();
    
    struct FactoryRiskData {
        uint32_t factory_id;
        float current_risk;
        float historical_average;
        uint64_t last_update;
        bool is_abnormal;
    };
    
    void processEvent(const FactoryEvent& event);
    void detectCrossFactoryAnomaly();
    
    std::vector<FactoryRiskData> factory_risk_data_;
    std::vector<FactoryEvent> recent_events_;
    std::vector<std::function<void(const FactoryEvent&)>> callbacks_;
    
    mutable std::mutex mutex_;
    std::atomic<float> global_risk_;
    std::atomic<size_t> anomaly_propagation_count_;
    
    static constexpr float GLOBAL_RISK_WEIGHT = 0.3f;
    static constexpr size_t MAX_EVENTS = 1000;
};