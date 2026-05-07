#include "tag_flow_tracker.h"

void TagFlowTracker::trackEvent(const RFIDEvent& event) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = tag_states_.find(event.tag_id);
    
    if (it != tag_states_.end()) {
        uint64_t prev_reader = it->second.last_reader;
        if (prev_reader != event.reader_id) {
            transitions_[prev_reader][event.reader_id]++;
        }
    }
    
    tag_states_[event.tag_id] = {event.reader_id, event.timestamp};
    reader_tag_counts_[event.reader_id]++;
}

float TagFlowTracker::getTransitionProbability(uint64_t from_reader, uint64_t to_reader) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto from_it = transitions_.find(from_reader);
    if (from_it == transitions_.end()) return 0.0f;
    
    auto to_it = from_it->second.find(to_reader);
    if (to_it == from_it->second.end()) return 0.0f;
    
    auto count_it = reader_tag_counts_.find(from_reader);
    if (count_it == reader_tag_counts_.end()) return 0.0f;
    
    return static_cast<float>(to_it->second) / count_it->second;
}

uint64_t TagFlowTracker::getTagCountThroughReader(uint64_t reader_id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = reader_tag_counts_.find(reader_id);
    return it != reader_tag_counts_.end() ? it->second : 0;
}

void TagFlowTracker::clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    tag_states_.clear();
    transitions_.clear();
    reader_tag_counts_.clear();
}