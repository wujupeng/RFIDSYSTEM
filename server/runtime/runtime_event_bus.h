#pragma once

#include <string>
#include <vector>
#include <mutex>
#include <functional>
#include <atomic>
#include <chrono>

enum class RuntimeEventType {
    GPU_RESET,
    REPLAY_SEEK,
    FRAME_DROP,
    DECISION_MISMATCH,
    BANDIT_COLLAPSE,
    BUFFER_OVERFLOW,
    SUBSCRIBER_SLOW,
    BUDGET_WARNING,
    VRAM_WARNING,
    CONNECTION_LOST
};

struct RuntimeEvent {
    RuntimeEventType type;
    uint64_t timestamp;
    uint64_t frame_id;
    std::string message;
    double value;
    
    RuntimeEvent() 
        : type(RuntimeEventType::FRAME_DROP), 
          timestamp(0), 
          frame_id(0),
          value(0.0) {}
};

class RuntimeEventBus {
public:
    using EventCallback = std::function<void(const RuntimeEvent&)>;
    
    static RuntimeEventBus& instance();
    
    void initialize();
    void shutdown();
    
    void subscribe(EventCallback callback);
    void unsubscribe(EventCallback callback);
    
    void publish(RuntimeEventType type, const std::string& message = "", double value = 0.0);
    void publishFrameEvent(RuntimeEventType type, uint64_t frameId, const std::string& message = "");
    
    const std::vector<RuntimeEvent>& getRecentEvents(size_t count = 100) const;
    
    size_t getEventCount(RuntimeEventType type) const;
    
private:
    RuntimeEventBus();
    
    std::vector<EventCallback> callbacks_;
    std::vector<RuntimeEvent> recent_events_;
    
    mutable std::mutex mutex_;
    
    std::atomic<uint64_t> event_counts_[10];
    
    static constexpr size_t MAX_RECENT_EVENTS = 1000;
};