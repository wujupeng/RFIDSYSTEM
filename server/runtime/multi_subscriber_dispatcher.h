#pragma once

#include "spatial_frame.h"
#include <vector>
#include <mutex>
#include <atomic>
#include <functional>
#include <string>

class MultiSubscriberDispatcher {
public:
    using SubscriberId = uint64_t;
    using FrameCallback = std::function<void(const SpatialFrame&)>;
    
    static MultiSubscriberDispatcher& instance();
    
    SubscriberId subscribe(const std::string& name, FrameCallback callback);
    void unsubscribe(SubscriberId id);
    
    void broadcast(const SpatialFrame& frame);
    
    size_t getSubscriberCount() const;
    
    void enableBackpressure(bool enabled);
    void setMaxPendingFrames(size_t max);
    
private:
    MultiSubscriberDispatcher();
    
    struct Subscriber {
        SubscriberId id;
        std::string name;
        FrameCallback callback;
        size_t pending_frames;
        std::atomic<bool> slow;
    };
    
    std::vector<Subscriber> subscribers_;
    mutable std::mutex mutex_;
    
    std::atomic<uint64_t> next_subscriber_id_;
    std::atomic<bool> backpressure_enabled_;
    std::atomic<size_t> max_pending_frames_;
};