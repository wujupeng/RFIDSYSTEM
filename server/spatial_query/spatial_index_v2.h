#pragma once

#include <cstdint>
#include <vector>
#include <map>

namespace spatial_query {

struct SpatialPoint {
    uint64_t id;
    float x;
    float y;
    float z;
    uint64_t timestamp;
};

struct SpatialRect {
    float min_x;
    float max_x;
    float min_y;
    float max_y;
};

struct SpatialCircle {
    float center_x;
    float center_y;
    float radius;
};

class SpatialIndexV2 {
public:
    static SpatialIndexV2& instance();
    
    void insert(const SpatialPoint& point);
    
    void remove(uint64_t id);
    
    void update(uint64_t id, float x, float y, float z);
    
    std::vector<SpatialPoint> queryRect(const SpatialRect& rect) const;
    
    std::vector<SpatialPoint> queryCircle(const SpatialCircle& circle) const;
    
    std::vector<SpatialPoint> queryNearby(float x, float y, float radius) const;
    
    size_t size() const;
    
    void clear();
    
private:
    SpatialIndexV2() = default;
    
    std::vector<SpatialPoint> points_;
};

} // namespace spatial_query