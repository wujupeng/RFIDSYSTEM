#pragma once

#include "../gpu/gpu_types.h"
#include "../../../server/runtime/spatial_frame.h"
#include <vector>
#include <unordered_map>
#include <memory>

struct DeltaChunk {
    uint32_t asset_id;
    uint32_t flags;
    
    float x;
    float y;
    float vx;
    float vy;
    float risk;
    float size;
};

class SpatialDeltaEncoder {
public:
    SpatialDeltaEncoder();
    ~SpatialDeltaEncoder();
    
    void initialize();
    
    std::vector<DeltaChunk> encode(const SpatialFrame& currentFrame);
    
    void reset();
    
    size_t getCompressionRatio() const;
    
private:
    std::unordered_map<int, AssetPosition> previous_assets_;
    
    bool hasChanged(const AssetPosition& prev, const AssetPosition& curr) const;
    
    size_t total_bytes_;
    size_t compressed_bytes_;
};