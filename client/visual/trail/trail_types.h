#pragma once

#include <cstdint>

struct GPUTrailPoint {
    float x;
    float y;
    float timestamp;
    float intensity;
    uint32_t asset_id;
    uint32_t color;
};

struct TrailConfig {
    size_t max_points;
    size_t max_assets;
    float decay_factor;
    float max_age;
    float intensity_scale;
};

struct TrailStats {
    size_t total_points;
    size_t active_assets;
    float memory_usage_mb;
    float avg_points_per_asset;
};