#include "spatial_index_v2.h"

namespace spatial_query {

SpatialIndexV2& SpatialIndexV2::instance() {
    static SpatialIndexV2 index;
    return index;
}

void SpatialIndexV2::insert(const SpatialPoint& point) {
    points_.push_back(point);
}

void SpatialIndexV2::remove(uint64_t id) {
    auto it = std::remove_if(points_.begin(), points_.end(),
        [id](const SpatialPoint& p) { return p.id == id; });
    points_.erase(it, points_.end());
}

void SpatialIndexV2::update(uint64_t id, float x, float y, float z) {
    for (auto& point : points_) {
        if (point.id == id) {
            point.x = x;
            point.y = y;
            point.z = z;
            point.timestamp = 0;
            break;
        }
    }
}

std::vector<SpatialPoint> SpatialIndexV2::queryRect(const SpatialRect& rect) const {
    std::vector<SpatialPoint> result;
    for (const auto& point : points_) {
        if (point.x >= rect.min_x && point.x <= rect.max_x &&
            point.y >= rect.min_y && point.y <= rect.max_y) {
            result.push_back(point);
        }
    }
    return result;
}

std::vector<SpatialPoint> SpatialIndexV2::queryCircle(const SpatialCircle& circle) const {
    std::vector<SpatialPoint> result;
    for (const auto& point : points_) {
        float dx = point.x - circle.center_x;
        float dy = point.y - circle.center_y;
        float dist_sq = dx * dx + dy * dy;
        if (dist_sq <= circle.radius * circle.radius) {
            result.push_back(point);
        }
    }
    return result;
}

std::vector<SpatialPoint> SpatialIndexV2::queryNearby(float x, float y, float radius) const {
    SpatialCircle circle;
    circle.center_x = x;
    circle.center_y = y;
    circle.radius = radius;
    return queryCircle(circle);
}

size_t SpatialIndexV2::size() const {
    return points_.size();
}

void SpatialIndexV2::clear() {
    points_.clear();
}

} // namespace spatial_query