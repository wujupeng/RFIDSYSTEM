#pragma once

#include "topology_types.h"
#include <unordered_map>
#include <mutex>

class TagFlowTracker {
public:
    void trackEvent(const RFIDEvent& event);
    
    float getTransitionProbability(uint64_t from_reader, uint64_t to_reader) const;
    
    uint64_t getTagCountThroughReader(uint64_t reader_id) const;
    
    void clear();
    
private:
    struct TagState {
        uint64_t last_reader;
        uint64_t last_timestamp;
    };
    
    std::unordered_map<uint64_t, TagState> tag_states_;
    std::unordered_map<uint64_t, std::unordered_map<uint64_t, uint64_t>> transitions_;
    std::unordered_map<uint64_t, uint64_t> reader_tag_counts_;
    
    mutable std::mutex mutex_;
};