#include "event_timeline.h"
#include <algorithm>

void EventTimeline::addEvent(const TimelineEvent& event) {
    events_.push_back(event);
}

std::vector<TimelineEvent> EventTimeline::getEventsByType(EventType type) {
    std::vector<TimelineEvent> result;
    for (const auto& event : events_) {
        if (event.type == type) {
            result.push_back(event);
        }
    }
    return result;
}

std::vector<TimelineEvent> EventTimeline::getEventsInTimeRange(uint64_t start, uint64_t end) {
    std::vector<TimelineEvent> result;
    for (const auto& event : events_) {
        if (event.timestamp >= start && event.timestamp <= end) {
            result.push_back(event);
        }
    }
    return result;
}

TimelineEvent EventTimeline::getEventById(uint64_t event_id) {
    for (const auto& event : events_) {
        if (event.event_id == event_id) {
            return event;
        }
    }
    return TimelineEvent();
}

size_t EventTimeline::getEventCount() const {
    return events_.size();
}

void EventTimeline::clear() {
    events_.clear();
}