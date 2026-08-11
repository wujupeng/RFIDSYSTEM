#pragma once

#include "ground_truth_point.h"
#include <vector>
#include <string>
#include <mutex>
#include <unordered_map>
#include <optional>

namespace pa::spatial_truth {

struct DedupResult {
    bool conflict = false;
    std::string existing_point_id;
    double existing_accuracy = 0.0;
};

class GroundTruthStore {
public:
    static GroundTruthStore& instance();

    bool addPoint(const GroundTruthPoint& point, DedupResult* dedup = nullptr);
    std::optional<GroundTruthPoint> getPoint(const std::string& point_id) const;
    std::vector<GroundTruthPoint> listByTagId(const std::string& tag_id) const;
    std::vector<GroundTruthPoint> listByCoordinateSystem(const std::string& cs_id) const;
    std::vector<GroundTruthPoint> listBySource(GroundTruthSource source) const;
    std::vector<GroundTruthPoint> listByTimeRange(uint64_t start_ts, uint64_t end_ts) const;
    std::vector<GroundTruthPoint> listByTagIdAndCoordinateSystem(const std::string& tag_id, const std::string& cs_id) const;
    bool removePoint(const std::string& point_id);
    size_t count() const;

    void setDedupWindowMs(uint64_t window_ms) { dedup_window_ms_ = window_ms; }

private:
    GroundTruthStore() = default;

    mutable std::mutex mutex_;
    std::unordered_map<std::string, GroundTruthPoint> points_by_id_;
    uint64_t dedup_window_ms_ = 1000;

    bool isDuplicateConflict(const GroundTruthPoint& existing, const GroundTruthPoint& incoming) const;
};

} // namespace pa::spatial_truth