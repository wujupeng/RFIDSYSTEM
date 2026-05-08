#include "offline_replay_queue.h"

namespace edge_runtime {

OfflineReplayQueue& OfflineReplayQueue::instance() {
    static OfflineReplayQueue queue;
    return queue;
}

void OfflineReplayQueue::initialize() {
    queue_.clear();
}

void OfflineReplayQueue::shutdown() {
    queue_.clear();
}

void OfflineReplayQueue::tick() {
}

void OfflineReplayQueue::enqueue(uint64_t timestamp, uint64_t frame_id, const std::string& data) {
    ReplayEntry entry;
    entry.entry_id = queue_.size() + 1;
    entry.timestamp = timestamp;
    entry.frame_id = frame_id;
    entry.data = data;
    
    queue_.push_back(entry);
}

bool OfflineReplayQueue::dequeue(ReplayEntry& entry) {
    if (queue_.empty()) {
        return false;
    }
    
    entry = queue_.front();
    queue_.erase(queue_.begin());
    return true;
}

bool OfflineReplayQueue::peek(ReplayEntry& entry) const {
    if (queue_.empty()) {
        return false;
    }
    
    entry = queue_.front();
    return true;
}

size_t OfflineReplayQueue::size() const {
    return queue_.size();
}

bool OfflineReplayQueue::isEmpty() const {
    return queue_.empty();
}

void OfflineReplayQueue::clear() {
    queue_.clear();
}

void OfflineReplayQueue::replayAll() {
    while (!queue_.empty()) {
        ReplayEntry entry;
        dequeue(entry);
    }
}

} // namespace edge_runtime