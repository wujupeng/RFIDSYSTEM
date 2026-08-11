#include "trail_collector.h"
#include <algorithm>

namespace pa::spatial_truth {

void TrailCollector::addPoint(const std::string& tag_id, const Point3D& point, uint64_t timestamp) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto& trail = trails_[tag_id];
    trail.tag_id = tag_id;
    trail.points.push_back(point);
    trail.timestamps.push_back(timestamp);

    uint64_t cutoff = (timestamp > time_window_ms_) ? (timestamp - time_window_ms_) : 0;
    while (!trail.timestamps.empty() && trail.timestamps.front() < cutoff) {
        trail.points.erase(trail.points.begin());
        trail.timestamps.erase(trail.timestamps.begin());
    }
}

std::vector<Trail> TrailCollector::getTrails(uint64_t time_window_ms) const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<Trail> result;
    for (const auto& [id, trail] : trails_) {
        if (trail.timestamps.empty()) continue;
        uint64_t latest = trail.timestamps.back();
        uint64_t cutoff = (latest > time_window_ms) ? (latest - time_window_ms) : 0;
        Trail filtered;
        filtered.tag_id = trail.tag_id;
        for (size_t i = 0; i < trail.timestamps.size(); ++i) {
            if (trail.timestamps[i] >= cutoff) {
                filtered.points.push_back(trail.points[i]);
                filtered.timestamps.push_back(trail.timestamps[i]);
            }
        }
        if (!filtered.points.empty()) result.push_back(filtered);
    }
    return result;
}

void FlowCollector::recordCrossing(const std::string& tag_id, const std::string& zone_id,
                                    CrossDirection direction, uint64_t timestamp) {
    std::lock_guard<std::mutex> lock(mutex_);
    Flow flow;
    flow.zone_id = zone_id;
    flow.timestamp = timestamp;
    if (direction == CrossDirection::IN) flow.in_count = 1;
    else flow.out_count = 1;
    flows_.push_back(flow);
}

std::vector<Flow> FlowCollector::getFlows(uint64_t time_window_ms) const {
    std::lock_guard<std::mutex> lock(mutex_);

    uint64_t now = 0;
    for (const auto& f : flows_) {
        if (f.timestamp > now) now = f.timestamp;
    }
    uint64_t cutoff = (now > time_window_ms) ? (now - time_window_ms) : 0;

    std::map<std::string, Flow> aggregated;
    for (const auto& f : flows_) {
        if (f.timestamp < cutoff) continue;
        auto& agg = aggregated[f.zone_id];
        agg.zone_id = f.zone_id;
        agg.in_count += f.in_count;
        agg.out_count += f.out_count;
        agg.timestamp = std::max(agg.timestamp, f.timestamp);
    }

    std::vector<Flow> result;
    for (const auto& [zone, flow] : aggregated) {
        result.push_back(flow);
    }
    return result;
}

} // namespace pa::spatial_truth