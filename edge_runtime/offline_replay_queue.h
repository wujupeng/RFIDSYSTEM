#pragma once

#include <cstdint>
#include <vector>
#include <map>

namespace edge_runtime {

struct ReplayEntry {
    uint64_t entry_id;
    uint64_t timestamp;
    uint64_t frame_id;
    std::string data;
};

class OfflineReplayQueue {
public:
    static OfflineReplayQueue& instance();
    
    void initialize();
    
    void shutdown();
    
    void tick();
    
    void enqueue(uint64_t timestamp, uint64_t frame_id, const std::string& data);
    
    bool dequeue(ReplayEntry& entry);
    
    bool peek(ReplayEntry& entry) const;
    
    size_t size() const;
    
    bool isEmpty() const;
    
    void clear();
    
    void replayAll();
    
private:
    OfflineReplayQueue() = default;
    
    std::vector<ReplayEntry> queue_;
};

} // namespace edge_runtime