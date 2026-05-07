#include "gpu_frame_uploader.h"
#include <cstring>
#include <algorithm>

GPUFrameUploader::GPUFrameUploader() 
    : max_assets_(10000), max_trails_(50000), max_heatmap_(4096), max_congestion_(256), max_overlays_(100),
      frame_hash_(0), frame_id_(0),
      asset_count_(0), trail_count_(0), heatmap_count_(0), congestion_count_(0), prediction_count_(0), overlay_count_(0),
      upload_mode_(UploadMode::PERSISTENT),
      total_uploaded_bytes_(0), upload_count_(0),
      initialized_(false) {
}

GPUFrameUploader::~GPUFrameUploader() {
    shutdown();
}

void GPUFrameUploader::initialize(size_t maxAssets, size_t maxTrails, 
                                 size_t maxHeatmap, size_t maxCongestion,
                                 size_t maxOverlays) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (initialized_) {
        shutdown();
    }
    
    max_assets_ = maxAssets;
    max_trails_ = maxTrails;
    max_heatmap_ = maxHeatmap;
    max_congestion_ = maxCongestion;
    max_overlays_ = maxOverlays;
    
    auto& pool = GPUBufferPool::instance();
    
    uint32_t persistentFlags = static_cast<uint32_t>(GPUBufferFlags::MAP_PERSISTENT) |
                               static_cast<uint32_t>(GPUBufferFlags::MAP_COHERENT);
    
    asset_buffer_ = pool.allocateBuffer(
        max_assets_ * sizeof(GPUAssetInstance),
        GPUBufferUsage::STREAM,
        persistentFlags
    );
    
    trail_buffer_ = pool.allocateBuffer(
        max_trails_ * sizeof(GPUTrailPoint),
        GPUBufferUsage::STREAM,
        persistentFlags
    );
    
    heatmap_buffer_ = pool.allocateBuffer(
        max_heatmap_ * sizeof(GPUHeatCell),
        GPUBufferUsage::STREAM,
        persistentFlags
    );
    
    congestion_buffer_ = pool.allocateBuffer(
        max_congestion_ * sizeof(GPUCongestionZone),
        GPUBufferUsage::STREAM,
        persistentFlags
    );
    
    prediction_buffer_ = pool.allocateBuffer(
        max_assets_ * sizeof(GPUPredictionPoint),
        GPUBufferUsage::STREAM,
        persistentFlags
    );
    
    overlay_buffer_ = pool.allocateBuffer(
        max_overlays_ * sizeof(GPUDecisionOverlay),
        GPUBufferUsage::STREAM,
        persistentFlags
    );
    
    initialized_ = true;
}

void GPUFrameUploader::shutdown() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        return;
    }
    
    auto& pool = GPUBufferPool::instance();
    
    if (asset_buffer_) {
        pool.releaseBuffer(asset_buffer_);
        asset_buffer_.reset();
    }
    
    if (trail_buffer_) {
        pool.releaseBuffer(trail_buffer_);
        trail_buffer_.reset();
    }
    
    if (heatmap_buffer_) {
        pool.releaseBuffer(heatmap_buffer_);
        heatmap_buffer_.reset();
    }
    
    if (congestion_buffer_) {
        pool.releaseBuffer(congestion_buffer_);
        congestion_buffer_.reset();
    }
    
    if (prediction_buffer_) {
        pool.releaseBuffer(prediction_buffer_);
        prediction_buffer_.reset();
    }
    
    if (overlay_buffer_) {
        pool.releaseBuffer(overlay_buffer_);
        overlay_buffer_.reset();
    }
    
    initialized_ = false;
}

void GPUFrameUploader::uploadFrame(const SpatialFrame& frame) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        return;
    }
    
    frame_id_ = frame.frame_id;
    frame_hash_ = frame.hash();
    
    uploadAssets(frame.assets);
    uploadTrails(frame.predictions);
    uploadHeatmap(frame.heatmap);
    uploadCongestion(frame.congestion);
    uploadPredictions(frame.predictions);
    uploadOverlays(frame.overlays);
    
    upload_count_++;
}

void GPUFrameUploader::uploadAssets(const std::vector<AssetPosition>& assets) {
    if (!asset_buffer_) {
        return;
    }
    
    size_t count = std::min(assets.size(), max_assets_);
    asset_count_ = static_cast<uint32_t>(count);
    
    void* ptr = asset_buffer_->map();
    if (!ptr) {
        return;
    }
    
    auto* instances = static_cast<GPUAssetInstance*>(ptr);
    
    for (size_t i = 0; i < count; ++i) {
        const auto& asset = assets[i];
        
        instances[i].x = static_cast<float>(asset.x);
        instances[i].y = static_cast<float>(asset.y);
        instances[i].velocity_x = static_cast<float>(asset.vx);
        instances[i].velocity_y = static_cast<float>(asset.vy);
        instances[i].risk = static_cast<float>(asset.risk_score);
        instances[i].color = getRiskColor(asset.risk_score);
        instances[i].size = asset.size;
        instances[i].heat = 0.0f;
        instances[i].flags = 0;
    }
    
    total_uploaded_bytes_ += count * sizeof(GPUAssetInstance);
}

void GPUFrameUploader::uploadTrails(const std::vector<PathPrediction>& predictions) {
    if (!trail_buffer_) {
        return;
    }
    
    size_t count = std::min(predictions.size(), max_trails_);
    trail_count_ = static_cast<uint32_t>(count);
    
    void* ptr = trail_buffer_->map();
    if (!ptr) {
        return;
    }
    
    auto* trails = static_cast<GPUTrailPoint*>(ptr);
    
    for (size_t i = 0; i < count; ++i) {
        const auto& pred = predictions[i];
        
        trails[i].x = static_cast<float>(pred.x);
        trails[i].y = static_cast<float>(pred.y);
        trails[i].timestamp = static_cast<float>(pred.timestamp) / 1000.0f;
        trails[i].intensity = static_cast<float>(pred.confidence);
    }
    
    total_uploaded_bytes_ += count * sizeof(GPUTrailPoint);
}

void GPUFrameUploader::uploadHeatmap(const std::vector<HeatCell>& heatmap) {
    if (!heatmap_buffer_) {
        return;
    }
    
    size_t count = std::min(heatmap.size(), max_heatmap_);
    heatmap_count_ = static_cast<uint32_t>(count);
    
    void* ptr = heatmap_buffer_->map();
    if (!ptr) {
        return;
    }
    
    auto* cells = static_cast<GPUHeatCell*>(ptr);
    
    for (size_t i = 0; i < count; ++i) {
        const auto& cell = heatmap[i];
        
        cells[i].x = static_cast<float>(cell.x);
        cells[i].y = static_cast<float>(cell.y);
        cells[i].value = cell.value;
        cells[i].padding = 0;
    }
    
    total_uploaded_bytes_ += count * sizeof(GPUHeatCell);
}

void GPUFrameUploader::uploadCongestion(const std::vector<CongestionZone>& congestion) {
    if (!congestion_buffer_) {
        return;
    }
    
    size_t count = std::min(congestion.size(), max_congestion_);
    congestion_count_ = static_cast<uint32_t>(count);
    
    void* ptr = congestion_buffer_->map();
    if (!ptr) {
        return;
    }
    
    auto* zones = static_cast<GPUCongestionZone*>(ptr);
    
    for (size_t i = 0; i < count; ++i) {
        const auto& zone = congestion[i];
        
        zones[i].x = zone.x;
        zones[i].y = zone.y;
        zones[i].radius = zone.radius;
        zones[i].density = zone.density;
        
        if (zone.level == "CONGESTED") {
            zones[i].color = packColor(1.0f, 0.23f, 0.19f, 0.4f);
        } else if (zone.level == "HIGH_DENSITY") {
            zones[i].color = packColor(1.0f, 0.69f, 0.13f, 0.3f);
        } else {
            zones[i].color = packColor(0.0f, 0.82f, 1.0f, 0.2f);
        }
        
        zones[i].flags = 0;
        zones[i].padding = 0;
    }
    
    total_uploaded_bytes_ += count * sizeof(GPUCongestionZone);
}

void GPUFrameUploader::uploadPredictions(const std::vector<PathPrediction>& predictions) {
    if (!prediction_buffer_) {
        return;
    }
    
    size_t count = std::min(predictions.size(), max_assets_);
    prediction_count_ = static_cast<uint32_t>(count);
    
    void* ptr = prediction_buffer_->map();
    if (!ptr) {
        return;
    }
    
    auto* points = static_cast<GPUPredictionPoint*>(ptr);
    
    for (size_t i = 0; i < count; ++i) {
        const auto& pred = predictions[i];
        
        points[i].x = static_cast<float>(pred.x);
        points[i].y = static_cast<float>(pred.y);
        points[i].confidence = static_cast<float>(pred.confidence);
        points[i].color = packColor(0.4f, 0.8f, 1.0f, 0.6f);
    }
    
    total_uploaded_bytes_ += count * sizeof(GPUPredictionPoint);
}

void GPUFrameUploader::uploadOverlays(const std::vector<DecisionOverlay>& overlays) {
    if (!overlay_buffer_) {
        return;
    }
    
    size_t count = std::min(overlays.size(), max_overlays_);
    overlay_count_ = static_cast<uint32_t>(count);
    
    void* ptr = overlay_buffer_->map();
    if (!ptr) {
        return;
    }
    
    auto* overlayData = static_cast<GPUDecisionOverlay*>(ptr);
    
    for (size_t i = 0; i < count; ++i) {
        const auto& overlay = overlays[i];
        
        overlayData[i].x = overlay.x;
        overlayData[i].y = overlay.y;
        overlayData[i].confidence = overlay.confidence;
        overlayData[i].action = overlay.action;
        overlayData[i].color = getActionColor(overlay.action);
        overlayData[i].pulse = 0.0f;
    }
    
    total_uploaded_bytes_ += count * sizeof(GPUDecisionOverlay);
}

uint32_t GPUFrameUploader::getRiskColor(double risk) const {
    if (risk < 0.5) {
        return packColor(0.0f, 0.82f, 1.0f, 1.0f);
    } else if (risk < 0.8) {
        return packColor(1.0f, 0.69f, 0.13f, 1.0f);
    } else {
        return packColor(1.0f, 0.23f, 0.19f, 1.0f);
    }
}

uint32_t GPUFrameUploader::getActionColor(uint32_t action) const {
    switch (action) {
        case 1: // INSPECT
            return packColor(1.0f, 0.2f, 0.2f, 0.8f);
        case 2: // ALERT
            return packColor(1.0f, 0.8f, 0.2f, 0.8f);
        case 3: // REALLOCATE
            return packColor(0.2f, 0.5f, 1.0f, 0.8f);
        default: // NO_ACTION
            return packColor(0.5f, 0.5f, 0.5f, 0.3f);
    }
}

uint64_t GPUFrameUploader::getFrameHash() const {
    return frame_hash_;
}

uint64_t GPUFrameUploader::getFrameId() const {
    return frame_id_;
}

uint32_t GPUFrameUploader::getAssetBufferId() const {
    return asset_buffer_ ? asset_buffer_->bufferId() : 0;
}

uint32_t GPUFrameUploader::getTrailBufferId() const {
    return trail_buffer_ ? trail_buffer_->bufferId() : 0;
}

uint32_t GPUFrameUploader::getHeatmapBufferId() const {
    return heatmap_buffer_ ? heatmap_buffer_->bufferId() : 0;
}

uint32_t GPUFrameUploader::getCongestionBufferId() const {
    return congestion_buffer_ ? congestion_buffer_->bufferId() : 0;
}

uint32_t GPUFrameUploader::getPredictionBufferId() const {
    return prediction_buffer_ ? prediction_buffer_->bufferId() : 0;
}

uint32_t GPUFrameUploader::getOverlayBufferId() const {
    return overlay_buffer_ ? overlay_buffer_->bufferId() : 0;
}

uint32_t GPUFrameUploader::getAssetCount() const {
    return asset_count_;
}

uint32_t GPUFrameUploader::getTrailCount() const {
    return trail_count_;
}

uint32_t GPUFrameUploader::getHeatmapCount() const {
    return heatmap_count_;
}

uint32_t GPUFrameUploader::getCongestionCount() const {
    return congestion_count_;
}

uint32_t GPUFrameUploader::getPredictionCount() const {
    return prediction_count_;
}

uint32_t GPUFrameUploader::getOverlayCount() const {
    return overlay_count_;
}

void GPUFrameUploader::setUploadMode(UploadMode mode) {
    upload_mode_ = mode;
}

GPUFrameUploader::UploadMode GPUFrameUploader::getUploadMode() const {
    return upload_mode_;
}

void GPUFrameUploader::flush() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (asset_buffer_) {
        asset_buffer_->unmap();
    }
    
    if (trail_buffer_) {
        trail_buffer_->unmap();
    }
    
    if (heatmap_buffer_) {
        heatmap_buffer_->unmap();
    }
    
    if (congestion_buffer_) {
        congestion_buffer_->unmap();
    }
    
    if (prediction_buffer_) {
        prediction_buffer_->unmap();
    }
    
    if (overlay_buffer_) {
        overlay_buffer_->unmap();
    }
}

size_t GPUFrameUploader::totalUploadedBytes() const {
    return total_uploaded_bytes_;
}

size_t GPUFrameUploader::uploadCount() const {
    return upload_count_;
}