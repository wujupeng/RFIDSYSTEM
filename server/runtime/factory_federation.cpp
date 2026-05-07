#include "factory_federation.h"
#include <algorithm>

FactoryFederation::FactoryFederation()
    : global_risk_(0.0f), anomaly_propagation_count_(0) {
}

FactoryFederation& FactoryFederation::instance() {
    static FactoryFederation instance;
    return instance;
}

void FactoryFederation::initialize() {
    global_risk_ = 0.0f;
    anomaly_propagation_count_ = 0;
}

void FactoryFederation::shutdown() {
    std::lock_guard<std::mutex> lock(mutex_);
    factory_risk_data_.clear();
    recent_events_.clear();
}

void FactoryFederation::registerFactory(uint32_t factoryId, const std::string& name, const std::string& location) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = std::find_if(factory_risk_data_.begin(), factory_risk_data_.end(),
                          [factoryId](const FactoryRiskData& f) { return f.factory_id == factoryId; });
    
    if (it == factory_risk_data_.end()) {
        FactoryRiskData data;
        data.factory_id = factoryId;
        data.current_risk = 0.0f;
        data.historical_average = 0.0f;
        data.last_update = 0;
        data.is_abnormal = false;
        
        factory_risk_data_.push_back(data);
        
        FactoryEvent event;
        event.type = FactoryEventType::FACTORY_ONLINE;
        event.factory_id = factoryId;
        event.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        event.risk_value = 0.0f;
        event.message = "Factory registered: " + name;
        
        processEvent(event);
    }
}

void FactoryFederation::unregisterFactory(uint32_t factoryId) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = std::find_if(factory_risk_data_.begin(), factory_risk_data_.end(),
                          [factoryId](const FactoryRiskData& f) { return f.factory_id == factoryId; });
    
    if (it != factory_risk_data_.end()) {
        FactoryEvent event;
        event.type = FactoryEventType::FACTORY_OFFLINE;
        event.factory_id = factoryId;
        event.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        event.risk_value = it->current_risk;
        event.message = "Factory unregistered";
        
        processEvent(event);
        
        factory_risk_data_.erase(it);
    }
}

void FactoryFederation::updateFactoryRisk(uint32_t factoryId, float risk, const AIFrameContext& context) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = std::find_if(factory_risk_data_.begin(), factory_risk_data_.end(),
                          [factoryId](const FactoryRiskData& f) { return f.factory_id == factoryId; });
    
    if (it != factory_risk_data_.end()) {
        float prevRisk = it->current_risk;
        it->current_risk = risk;
        it->last_update = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        
        it->historical_average = (it->historical_average * 9 + risk) / 10;
        
        float diff = std::abs(risk - it->historical_average);
        it->is_abnormal = (diff > 0.3f);
        
        if (risk > 0.7f) {
            FactoryEvent event;
            event.type = FactoryEventType::RISK_THRESHOLD_EXCEEDED;
            event.factory_id = factoryId;
            event.timestamp = it->last_update;
            event.risk_value = risk;
            event.message = "Risk threshold exceeded";
            
            processEvent(event);
            
            if (it->is_abnormal) {
                propagateAnomaly(factoryId);
            }
        }
    }
    
    float globalRisk = 0.0f;
    for (const auto& data : factory_risk_data_) {
        globalRisk += data.current_risk;
    }
    
    if (!factory_risk_data_.empty()) {
        globalRisk /= factory_risk_data_.size();
    }
    
    global_risk_ = globalRisk;
}

float FactoryFederation::getGlobalRisk() const {
    return global_risk_;
}

std::vector<uint32_t> FactoryFederation::getHighRiskFactories(float threshold) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::vector<uint32_t> result;
    for (const auto& data : factory_risk_data_) {
        if (data.current_risk > threshold) {
            result.push_back(data.factory_id);
        }
    }
    
    return result;
}

void FactoryFederation::propagateAnomaly(uint32_t sourceFactoryId) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    FactoryEvent event;
    event.type = FactoryEventType::ANOMALY_DETECTED;
    event.factory_id = sourceFactoryId;
    event.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    
    auto it = std::find_if(factory_risk_data_.begin(), factory_risk_data_.end(),
                          [sourceFactoryId](const FactoryRiskData& f) { return f.factory_id == sourceFactoryId; });
    
    if (it != factory_risk_data_) {
        event.risk_value = it->current_risk;
    }
    
    event.message = "Anomaly detected, propagating to connected factories";
    
    processEvent(event);
    
    for (auto& data : factory_risk_data_) {
        if (data.factory_id != sourceFactoryId) {
            data.current_risk = std::min(1.0f, data.current_risk + GLOBAL_RISK_WEIGHT * (event.risk_value - data.current_risk));
        }
    }
    
    anomaly_propagation_count_++;
}

void FactoryFederation::syncFactories() {
    FactoryEvent event;
    event.type = FactoryEventType::SYNC_REQUEST;
    event.factory_id = 0;
    event.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    event.risk_value = global_risk_;
    event.message = "Initiating event-based sync";
    
    processEvent(event);
    
    detectCrossFactoryAnomaly();
    
    event.type = FactoryEventType::SYNC_COMPLETE;
    event.message = "Sync completed";
    event.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    
    processEvent(event);
}

void FactoryFederation::registerEventCallback(std::function<void(const FactoryEvent&)> callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    callbacks_.push_back(callback);
}

const std::vector<FactoryEvent>& FactoryFederation::getRecentEvents(size_t count) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    static std::vector<FactoryEvent> result;
    result.clear();
    
    size_t start = count >= recent_events_.size() ? 0 : recent_events_.size() - count;
    for (size_t i = start; i < recent_events_.size(); ++i) {
        result.push_back(recent_events_[i]);
    }
    
    return result;
}

size_t FactoryFederation::getAnomalyPropagationCount() const {
    return anomaly_propagation_count_;
}

void FactoryFederation::processEvent(const FactoryEvent& event) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (recent_events_.size() >= MAX_EVENTS) {
        recent_events_.erase(recent_events_.begin());
    }
    
    recent_events_.push_back(event);
    
    for (const auto& callback : callbacks_) {
        try {
            callback(event);
        } catch (...) {
        }
    }
}

void FactoryFederation::detectCrossFactoryAnomaly() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    int abnormalCount = 0;
    for (const auto& data : factory_risk_data_) {
        if (data.is_abnormal) {
            abnormalCount++;
        }
    }
    
    if (abnormalCount > 1 && abnormalCount >= factory_risk_data_.size() / 2) {
        FactoryEvent event;
        event.type = FactoryEventType::ANOMALY_DETECTED;
        event.factory_id = 0;
        event.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        event.risk_value = global_risk_;
        event.message = "Cross-factory anomaly detected: multiple factories showing abnormal behavior";
        
        processEvent(event);
    }
}