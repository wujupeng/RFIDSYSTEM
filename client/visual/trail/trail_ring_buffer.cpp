#include "trail_ring_buffer.h"
#include <cstring>

TrailRingBuffer::TrailRingBuffer()
    : mapped_ptr_(nullptr), write_head_(0), read_head_(0), initialized_(false) {
}

TrailRingBuffer::~TrailRingBuffer() {
    shutdown();
}

void TrailRingBuffer::initialize(const TrailConfig& config) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (initialized_) {
        shutdown();
    }
    
    config_ = config;
    
    auto& pool = GPUBufferPool::instance();
    uint32_t flags = static_cast<uint32_t>(GPUBufferFlags::MAP_PERSISTENT) |
                     static_cast<uint32_t>(GPUBufferFlags::MAP_COHERENT);
    
    gpu_buffer_ = pool.allocateBuffer(
        config.max_points * sizeof(GPUTrailPoint),
        GPUBufferUsage::STREAM,
        flags
    );
    
    mapped_ptr_ = static_cast<GPUTrailPoint*>(gpu_buffer_->map());
    
    write_head_ = 0;
    read_head_ = 0;
    
    asset_point_counts_.reserve(config.max_assets);
    
    initialized_ = true;
}

void TrailRingBuffer::shutdown() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        return;
    }
    
    if (gpu_buffer_) {
        gpu_buffer_->unmap();
        auto& pool = GPUBufferPool::instance();
        pool.releaseBuffer(gpu_buffer_);
        gpu_buffer_.reset();
    }
    
    mapped_ptr_ = nullptr;
    write_head_ = 0;
    read_head_ = 0;
    asset_point_counts_.clear();
    asset_velocities_.clear();
    
    initialized_ = false;
}

void TrailRingBuffer::addPoint(uint32_t assetId, float x, float y, float timestamp,
                               float intensity, uint32_t color) {
    if (!initialized_ || !mapped_ptr_) {
        return;
    }
    
    size_t head = write_head_.fetch_add(1) % config_.max_points;
    
    mapped_ptr_[head].x = x;
    mapped_ptr_[head].y = y;
    mapped_ptr_[head].timestamp = timestamp;
    mapped_ptr_[head].intensity = intensity;
    mapped_ptr_[head].asset_id = assetId;
    mapped_ptr_[head].color = color;
    
    std::lock_guard<std::mutex> lock(mutex_);
    asset_point_counts_[assetId]++;
}

void TrailRingBuffer::addPoints(uint32_t assetId, const std::vector<GPUTrailPoint>& points) {
    if (!initialized_ || !mapped_ptr_ || points.empty()) {
        return;
    }
    
    for (const auto& point : points) {
        size_t head = write_head_.fetch_add(1) % config_.max_points;
        mapped_ptr_[head] = point;
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    asset_point_counts_[assetId] += points.size();
}

void TrailRingBuffer::updateAssetVelocity(uint32_t assetId, float vx, float vy) {
    std::lock_guard<std::mutex> lock(mutex_);
    asset_velocities_[assetId] = {vx, vy};
}

void TrailRingBuffer::cleanupOldPoints(float currentTime) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    size_t read = read_head_.load();
    size_t write = write_head_.load();
    
    while (read != write) {
        if (currentTime - mapped_ptr_[read].timestamp > config_.max_age) {
            uint32_t assetId = mapped_ptr_[read].asset_id;
            auto it = asset_point_counts_.find(assetId);
            if (it != asset_point_counts_.end() && it->second > 0) {
                it->second--;
            }
            
            read_head_.store((read + 1) % config_.max_points);
            read = read_head_.load();
        } else {
            break;
        }
    }
}

uint32_t TrailRingBuffer::getBufferId() const {
    return gpu_buffer_ ? gpu_buffer_->bufferId() : 0;
}

size_t TrailRingBuffer::getPointCount() const {
    size_t write = write_head_.load();
    size_t read = read_head_.load();
    
    if (write >= read) {
        return write - read;
    }
    
    return config_.max_points - read + write;
}

const TrailConfig& TrailRingBuffer::getConfig() const {
    return config_;
}

TrailStats TrailRingBuffer::getStats() const {
    TrailStats stats;
    stats.total_points = getPointCount();
    stats.active_assets = asset_point_counts_.size();
    stats.memory_usage_mb = static_cast<float>(stats.total_points * sizeof(GPUTrailPoint)) / (1024.0f * 1024.0f);
    
    if (stats.active_assets > 0) {
        stats.avg_points_per_asset = static_cast<float>(stats.total_points) / stats.active_assets;
    } else {
        stats.avg_points_per_asset = 0.0f;
    }
    
    return stats;
}

void TrailRingBuffer::flush() {
    if (gpu_buffer_) {
        gpu_buffer_->unmap();
        gpu_buffer_->map();
    }
}