#include "ground_truth_store.h"
#include <algorithm>

namespace pa::spatial_truth {

GroundTruthStore& GroundTruthStore::instance() {
    static GroundTruthStore inst;
    return inst;
}

bool GroundTruthStore::isDuplicateConflict(const GroundTruthPoint& existing, const GroundTruthPoint& incoming) const {
    if (existing.tag_id != incoming.tag_id) return false;
    if (existing.coordinate_system_id != incoming.coordinate_system_id) return false;
    uint64_t diff = (existing.timestamp > incoming.timestamp)
                     ? (existing.timestamp - incoming.timestamp)
                     : (incoming.timestamp - existing.timestamp);
    return diff <= dedup_window_ms_;
}

bool GroundTruthStore::addPoint(const GroundTruthPoint& point, DedupResult* dedup) {
    std::lock_guard<std::mutex> lock(mutex_);

    for (const auto& [id, existing] : points_by_id_) {
        if (isDuplicateConflict(existing, point)) {
            if (dedup) {
                dedup->conflict = true;
                dedup->existing_point_id = id;
                dedup->existing_accuracy = existing.accuracy;
            }
            if (point.accuracy >= existing.accuracy) {
                return false;
            }
            break;
        }
    }

    points_by_id_[point.point_id] = point;
    return true;
}

std::optional<GroundTruthPoint> GroundTruthStore::getPoint(const std::string& point_id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = points_by_id_.find(point_id);
    if (it == points_by_id_.end()) return std::nullopt;
    return it->second;
}

std::vector<GroundTruthPoint> GroundTruthStore::listByTagId(const std::string& tag_id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<GroundTruthPoint> result;
    for (const auto& [id, p] : points_by_id_) {
        if (p.tag_id == tag_id) result.push_back(p);
    }
    return result;
}

std::vector<GroundTruthPoint> GroundTruthStore::listByCoordinateSystem(const std::string& cs_id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<GroundTruthPoint> result;
    for (const auto& [id, p] : points_by_id_) {
        if (p.coordinate_system_id == cs_id) result.push_back(p);
    }
    return result;
}

std::vector<GroundTruthPoint> GroundTruthStore::listBySource(GroundTruthSource source) const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<GroundTruthPoint> result;
    for (const auto& [id, p] : points_by_id_) {
        if (p.source == source) result.push_back(p);
    }
    return result;
}

std::vector<GroundTruthPoint> GroundTruthStore::listByTimeRange(uint64_t start_ts, uint64_t end_ts) const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<GroundTruthPoint> result;
    for (const auto& [id, p] : points_by_id_) {
        if (p.timestamp >= start_ts && p.timestamp <= end_ts) result.push_back(p);
    }
    return result;
}

std::vector<GroundTruthPoint> GroundTruthStore::listByTagIdAndCoordinateSystem(const std::string& tag_id, const std::string& cs_id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<GroundTruthPoint> result;
    for (const auto& [id, p] : points_by_id_) {
        if (p.tag_id == tag_id && p.coordinate_system_id == cs_id) result.push_back(p);
    }
    return result;
}

bool GroundTruthStore::removePoint(const std::string& point_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    return points_by_id_.erase(point_id) > 0;
}

size_t GroundTruthStore::count() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return points_by_id_.size();
}

} // namespace pa::spatial_truth