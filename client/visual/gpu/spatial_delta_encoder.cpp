#include "spatial_delta_encoder.h"
#include <cmath>

SpatialDeltaEncoder::SpatialDeltaEncoder()
    : total_bytes_(0), compressed_bytes_(0) {
}

SpatialDeltaEncoder::~SpatialDeltaEncoder() {
}

void SpatialDeltaEncoder::initialize() {
    previous_assets_.clear();
    total_bytes_ = 0;
    compressed_bytes_ = 0;
}

std::vector<DeltaChunk> SpatialDeltaEncoder::encode(const SpatialFrame& currentFrame) {
    std::vector<DeltaChunk> delta;
    
    for (const auto& asset : currentFrame.assets) {
        auto it = previous_assets_.find(asset.asset_id);
        
        if (it == previous_assets_.end() || hasChanged(it->second, asset)) {
            DeltaChunk chunk;
            chunk.asset_id = static_cast<uint32_t>(asset.asset_id);
            chunk.flags = 0;
            
            if (it == previous_assets_.end()) {
                chunk.flags |= 0x01;
            }
            
            chunk.x = static_cast<float>(asset.x);
            chunk.y = static_cast<float>(asset.y);
            chunk.vx = static_cast<float>(asset.vx);
            chunk.vy = static_cast<float>(asset.vy);
            chunk.risk = static_cast<float>(asset.risk_score);
            chunk.size = asset.size;
            
            delta.push_back(chunk);
        }
        
        previous_assets_[asset.asset_id] = asset;
    }
    
    total_bytes_ += currentFrame.assets.size() * sizeof(GPUAssetInstance);
    compressed_bytes_ += delta.size() * sizeof(DeltaChunk);
    
    return delta;
}

bool SpatialDeltaEncoder::hasChanged(const AssetPosition& prev, const AssetPosition& curr) const {
    const float epsilon = 0.01f;
    
    if (std::abs(static_cast<float>(prev.x - curr.x)) > epsilon) return true;
    if (std::abs(static_cast<float>(prev.y - curr.y)) > epsilon) return true;
    if (std::abs(static_cast<float>(prev.vx - curr.vx)) > epsilon) return true;
    if (std::abs(static_cast<float>(prev.vy - curr.vy)) > epsilon) return true;
    if (std::abs(static_cast<float>(prev.risk_score - curr.risk_score)) > 0.001f) return true;
    if (prev.size != curr.size) return true;
    
    return false;
}

void SpatialDeltaEncoder::reset() {
    previous_assets_.clear();
    total_bytes_ = 0;
    compressed_bytes_ = 0;
}

size_t SpatialDeltaEncoder::getCompressionRatio() const {
    if (total_bytes_ == 0) {
        return 0;
    }
    
    return (total_bytes_ * 100) / compressed_bytes_;
}