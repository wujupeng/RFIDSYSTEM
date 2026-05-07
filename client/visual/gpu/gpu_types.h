#pragma once

#include <cstdint>
#include <cstring>

struct GPUAssetInstance {
    float x;
    float y;
    
    float velocity_x;
    float velocity_y;
    
    float risk;
    
    uint32_t color;
    
    float size;
    
    float heat;
    
    uint32_t flags;
};

struct GPUTrailPoint {
    float x;
    float y;
    float timestamp;
    float intensity;
};

struct GPUHeatCell {
    float x;
    float y;
    float value;
    uint32_t padding;
};

struct GPUCongestionZone {
    float x;
    float y;
    float radius;
    float density;
    
    uint32_t color;
    uint32_t flags;
    uint32_t padding;
};

struct GPUPredictionPoint {
    float x;
    float y;
    float confidence;
    uint32_t color;
};

struct GPUDecisionOverlay {
    float x;
    float y;
    
    float confidence;
    
    uint32_t action;
    uint32_t color;
    
    float pulse;
};

struct GPUVelocityField {
    float vx;
    float vy;
    float density;
};

struct GPUFrameData {
    uint64_t frame_id;
    uint64_t timestamp;
    
    uint32_t asset_count;
    uint32_t trail_count;
    uint32_t heatmap_count;
    uint32_t congestion_count;
    uint32_t prediction_count;
    uint32_t padding[3];
    
    uint64_t frame_hash;
};

struct GPUBufferConfig {
    size_t size;
    uint32_t usage;
    uint32_t flags;
    
    void* mapped_ptr;
    uint32_t buffer_id;
};

enum class GPUBufferUsage {
    STATIC,
    DYNAMIC,
    STREAM
};

enum class GPUBufferFlags {
    NONE = 0,
    MAP_PERSISTENT = 1,
    MAP_COHERENT = 2,
    MAP_WRITE_INVALIDATE = 4
};

inline uint32_t packColor(float r, float g, float b, float a = 1.0f) {
    return (static_cast<uint32_t>(r * 255.0f) << 24) |
           (static_cast<uint32_t>(g * 255.0f) << 16) |
           (static_cast<uint32_t>(b * 255.0f) << 8) |
           (static_cast<uint32_t>(a * 255.0f));
}

inline void unpackColor(uint32_t packed, float& r, float& g, float& b, float& a) {
    r = ((packed >> 24) & 0xFF) / 255.0f;
    g = ((packed >> 16) & 0xFF) / 255.0f;
    b = ((packed >> 8) & 0xFF) / 255.0f;
    a = (packed & 0xFF) / 255.0f;
}