#include "multi_subscriber_dispatcher.h"

MultiSubscriberDispatcher::MultiSubscriberDispatcher()
    : next_subscriber_id_(1), backpressure_enabled_(true), max_pending_frames_(100) {
}

MultiSubscriberDispatcher& MultiSubscriberDispatcher::instance() {
    static MultiSubscriberDispatcher instance;
    return instance;
}

MultiSubscriberDispatcher::SubscriberId MultiSubscriberDispatcher::subscribe(const std::string& name, FrameCallback callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    Subscriber subscriber;
    subscriber.id = next_subscriber_id_++;
    subscriber.name = name;
    subscriber.callback = callback;
    subscriber.pending_frames = 0;
    subscriber.slow = false;
    
    subscribers_.push_back(subscriber);
    
    return subscriber.id;
}

void MultiSubscriberDispatcher::unsubscribe(SubscriberId id) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = std::find_if(subscribers_.begin(), subscribers_.end(),
                          [id](const Subscriber& s) { return s.id == id; });
    
    if (it != subscribers_.end()) {
        subscribers_.erase(it);
    }
}

void MultiSubscriberDispatcher::broadcast(const SpatialFrame& frame) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    for (auto& subscriber : subscribers_) {
        if (backpressure_enabled_ && subscriber.pending_frames > max_pending_frames_) {
            subscriber.slow = true;
            continue;
        }
        
        subscriber.pending_frames++;
        
        try {
            subscriber.callback(frame);
            subscriber.pending_frames--;
            subscriber.slow = false;
        } catch (...) {
            subscriber.pending_frames--;
        }
    }
}

size_t MultiSubscriberDispatcher::getSubscriberCount() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return subscribers_.size();
}

void MultiSubscriberDispatcher::enableBackpressure(bool enabled) {
    backpressure_enabled_ = enabled;
}

void MultiSubscriberDispatcher::setMaxPendingFrames(size_t max) {
    max_pending_frames_ = max;
}