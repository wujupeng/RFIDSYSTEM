#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <map>

enum class EventType {
    POLICY_CHANGE,
    RECOVERY_ACTION,
    FAILURE_DETECTED,
    TOPOLOGY_MODIFICATION,
    AI_DECISION,
    GOVERNANCE_VIOLATION,
    SYSTEM_UPGRADE
};

struct TimelineEvent {
    uint64_t event_id;
    uint64_t timestamp;
    EventType type;
    std::string description;
    std::string actor;
    uint64_t target_id;
    std::map<std::string, std::string> metadata;
};

class EventTimeline {
public:
    void addEvent(const TimelineEvent& event);
    
    std::vector<TimelineEvent> getEventsByType(EventType type);
    
    std::vector<TimelineEvent> getEventsInTimeRange(uint64_t start, uint64_t end);
    
    TimelineEvent getEventById(uint64_t event_id);
    
    size_t getEventCount() const;
    
    void clear();
    
private:
    std::vector<TimelineEvent> events_;
};