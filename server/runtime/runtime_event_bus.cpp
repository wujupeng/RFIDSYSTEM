#include "runtime_event_bus.h"
#include <algorithm>

RuntimeEventBus::RuntimeEventBus() {
    for (int i = 0; i < 10; ++i) {
        event_counts_[i] = 0;
    }
}

RuntimeEventBus& RuntimeEventBus::instance() {
    static RuntimeEventBus instance;
    return instance;
}

void RuntimeEventBus::initialize() {
    recent_events_.clear();
    for (int i = 0; i < 10; ++i) {
        event_counts_[i] = 0;
    }
}

void RuntimeEventBus::shutdown() {
    std::lock_guard<std::mutex> lock(mutex_);
    callbacks_.clear();
    recent_events_.clear();
}

void RuntimeEventBus::subscribe(EventCallback callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    callbacks_.push_back(callback);
}

void RuntimeEventBus::unsubscribe(EventCallback callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = std::find(callbacks_.begin(), callbacks_.end(), callback);
    if (it != callbacks_.end()) {
        callbacks_.erase(it);
    }
}

void RuntimeEventBus::publish(RuntimeEventType type, const std::string& message, double value) {
    uint64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    
    RuntimeEvent event;
    event.type = type;
    event.timestamp = now;
    event.frame_id = 0;
    event.message = message;
    event.value = value;
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (recent_events_.size() >= MAX_RECENT_EVENTS) {
        recent_events_.erase(recent_events_.begin());
    }
    recent_events_.push_back(event);
    
    event_counts_[static_cast<int>(type)]++;
    
    for (const auto& callback : callbacks_) {
        try {
            callback(event);
        } catch (...) {
        }
    }
}

void RuntimeEventBus::publishFrameEvent(RuntimeEventType type, uint64_t frameId, const std::string& message) {
    uint64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    
    RuntimeEvent event;
    event.type = type;
    event.timestamp = now;
    event.frame_id = frameId;
    event.message = message;
    event.value = 0.0;
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (recent_events_.size() >= MAX_RECENT_EVENTS) {
        recent_events_.erase(recent_events_.begin());
    }
    recent_events_.push_back(event);
    
    event_counts_[static_cast<int>(type)]++;
    
    for (const auto& callback : callbacks_) {
        try {
            callback(event);
        } catch (...) {
        }
    }
}

const std::vector<RuntimeEvent>& RuntimeEventBus::getRecentEvents(size_t count) const {
    static std::vector<RuntimeEvent> empty;
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (count >= recent_events_.size()) {
        return recent_events_;
    }
    
    static std::vector<RuntimeEvent> result;
    result.clear();
    result.insert(result.begin(), 
                  recent_events_.end() - static_cast<int>(count), 
                  recent_events_.end());
    
    return result;
}

size_t RuntimeEventBus::getEventCount(RuntimeEventType type) const {
    return event_counts_[static_cast<int>(type)];
}