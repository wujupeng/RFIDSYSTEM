#pragma once

#include "factory_map.h"
#include <vector>
#include <string>
#include <map>
#include <mutex>

namespace pa::spatial_truth {

class TrailCollector {
public:
    void addPoint(const std::string& tag_id, const Point3D& point, uint64_t timestamp);
    std::vector<Trail> getTrails(uint64_t time_window_ms) const;
    void setTimeWindowMs(uint64_t window) { time_window_ms_ = window; }

private:
    mutable std::mutex mutex_;
    std::map<std::string, Trail> trails_;
    uint64_t time_window_ms_ = 60000;
};

enum class CrossDirection { IN, OUT };

class FlowCollector {
public:
    void recordCrossing(const std::string& tag_id, const std::string& zone_id,
                        CrossDirection direction, uint64_t timestamp);
    std::vector<Flow> getFlows(uint64_t time_window_ms) const;

private:
    mutable std::mutex mutex_;
    std::vector<Flow> flows_;
};

} // namespace pa::spatial_truth