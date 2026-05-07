#pragma once

#include "gpu_types.h"
#include "gpu_buffer_pool.h"
#include "../../../server/runtime/spatial_frame.h"
#include <memory>
#include <mutex>
#include <atomic>

class GPUFrameUploader {
public:
    enum class UploadMode {
        SYNC,
        ASYNC,
        PERSISTENT
    };
    
    GPUFrameUploader();
    ~GPUFrameUploader();
    
    void initialize(size_t maxAssets = 10000, 
                   size_t maxTrails = 50000,
                   size_t maxHeatmap = 4096,
                   size_t maxCongestion = 256,
                   size_t maxOverlays = 100);
    
    void shutdown();
    
    void uploadFrame(const SpatialFrame& frame);
    
    uint64_t getFrameHash() const;
    uint64_t getFrameId() const;
    
    uint32_t getAssetBufferId() const;
    uint32_t getTrailBufferId() const;
    uint32_t getHeatmapBufferId() const;
    uint32_t getCongestionBufferId() const;
    uint32_t getPredictionBufferId() const;
    uint32_t getOverlayBufferId() const;
    
    uint32_t getAssetCount() const;
    uint32_t getTrailCount() const;
    uint32_t getHeatmapCount() const;
    uint32_t getCongestionCount() const;
    uint32_t getPredictionCount() const;
    uint32_t getOverlayCount() const;
    
    void setUploadMode(UploadMode mode);
    UploadMode getUploadMode() const;
    
    void flush();
    
    size_t totalUploadedBytes() const;
    size_t uploadCount() const;
    
private:
    void uploadAssets(const std::vector<AssetPosition>& assets);
    void uploadTrails(const std::vector<PathPrediction>& predictions);
    void uploadHeatmap(const std::vector<HeatCell>& heatmap);
    void uploadCongestion(const std::vector<CongestionZone>& congestion);
    void uploadPredictions(const std::vector<PathPrediction>& predictions);
    void uploadOverlays(const std::vector<DecisionOverlay>& overlays);
    
    uint32_t getRiskColor(double risk) const;
    uint32_t getActionColor(uint32_t action) const;
    
    std::mutex mutex_;
    
    std::shared_ptr<GPUBuffer> asset_buffer_;
    std::shared_ptr<GPUBuffer> trail_buffer_;
    std::shared_ptr<GPUBuffer> heatmap_buffer_;
    std::shared_ptr<GPUBuffer> congestion_buffer_;
    std::shared_ptr<GPUBuffer> prediction_buffer_;
    std::shared_ptr<GPUBuffer> overlay_buffer_;
    
    size_t max_assets_;
    size_t max_trails_;
    size_t max_heatmap_;
    size_t max_congestion_;
    size_t max_overlays_;
    
    std::atomic<uint64_t> frame_hash_;
    std::atomic<uint64_t> frame_id_;
    
    std::atomic<uint32_t> asset_count_;
    std::atomic<uint32_t> trail_count_;
    std::atomic<uint32_t> heatmap_count_;
    std::atomic<uint32_t> congestion_count_;
    std::atomic<uint32_t> prediction_count_;
    std::atomic<uint32_t> overlay_count_;
    
    UploadMode upload_mode_;
    
    std::atomic<size_t> total_uploaded_bytes_;
    std::atomic<size_t> upload_count_;
    
    bool initialized_;
};