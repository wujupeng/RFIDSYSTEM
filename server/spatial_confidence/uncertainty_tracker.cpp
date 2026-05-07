#include "uncertainty_tracker.h"
#include <cmath>
#include <algorithm>

UncertaintyTracker::UncertaintyTracker() {}

UncertaintyTracker& UncertaintyTracker::instance() {
    static UncertaintyTracker instance;
    return instance;
}

void UncertaintyTracker::update(const std::string& epc, const TagPosition& pos) {
    history_[epc].push_back(pos);
    
    if (history_[epc].size() > 20) {
        history_[epc].erase(history_[epc].begin());
    }
}

UncertaintyBounds UncertaintyTracker::getBounds(const std::string& epc) {
    UncertaintyBounds bounds;
    bounds.min_x = bounds.max_x = bounds.min_y = bounds.max_y = 0.0;
    bounds.std_dev = 0.0;
    
    auto it = history_.find(epc);
    if (it == history_.end() || it->second.empty()) {
        return bounds;
    }
    
    const auto& positions = it->second;
    
    bounds.min_x = positions[0].x;
    bounds.max_x = positions[0].x;
    bounds.min_y = positions[0].y;
    bounds.max_y = positions[0].y;
    
    for (const auto& pos : positions) {
        bounds.min_x = std::min(bounds.min_x, pos.x);
        bounds.max_x = std::max(bounds.max_x, pos.x);
        bounds.min_y = std::min(bounds.min_y, pos.y);
        bounds.max_y = std::max(bounds.max_y, pos.y);
    }
    
    double mean_x = 0.0, mean_y = 0.0;
    for (const auto& pos : positions) {
        mean_x += pos.x;
        mean_y += pos.y;
    }
    mean_x /= positions.size();
    mean_y /= positions.size();
    
    double variance = 0.0;
    for (const auto& pos : positions) {
        double dx = pos.x - mean_x;
        double dy = pos.y - mean_y;
        variance += dx * dx + dy * dy;
    }
    variance /= positions.size();
    bounds.std_dev = sqrt(variance);
    
    return bounds;
}

double UncertaintyTracker::getUncertainty(const std::string& epc) {
    auto bounds = getBounds(epc);
    return bounds.std_dev;
}

void UncertaintyTracker::reset(const std::string& epc) {
    history_.erase(epc);
}