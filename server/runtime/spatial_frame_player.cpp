#include "spatial_frame_player.h"
#include "spatial_frame.h"
#include "spatial_frame_recorder.h"
#include "protocol/spatial_frame.pb.h"
#include <cstring>
#include <chrono>
#include <algorithm>

SpatialFramePlayer::SpatialFramePlayer() 
    : state_(PlaybackState::STOPPED), speed_(1.0), 
      currentFrameIndex_(0), lastFrameHash_(0), running_(false) {
}

SpatialFramePlayer::~SpatialFramePlayer() {
    close();
}

bool SpatialFramePlayer::open(const std::string& path) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (file_.is_open()) {
        close();
    }
    
    file_.open(path, std::ios::binary);
    if (!file_.is_open()) {
        return false;
    }
    
    loadIndex();
    
    return true;
}

void SpatialFramePlayer::close() {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (running_) {
            running_ = false;
            queueCondition_.notify_all();
        }
    }
    
    if (playbackThread_.joinable()) {
        playbackThread_.join();
    }
    
    if (file_.is_open()) {
        file_.close();
    }
    
    framePositions_.clear();
    frameTimestamps_.clear();
    state_ = PlaybackState::STOPPED;
    currentFrameIndex_ = 0;
}

bool SpatialFramePlayer::isOpen() const {
    return file_.is_open();
}

void SpatialFramePlayer::loadIndex() {
    framePositions_.clear();
    frameTimestamps_.clear();
    
    file_.seekg(0);
    
    FrameRecordHeader header;
    while (readHeader(file_, header)) {
        framePositions_.push_back(file_.tellg() - sizeof(header));
        frameTimestamps_.push_back(header.timestamp);
        
        file_.seekg(header.payload_size, std::ios::cur);
    }
    
    file_.clear();
    file_.seekg(0);
}

bool SpatialFramePlayer::readHeader(std::ifstream& file, FrameRecordHeader& header) {
    file.read(reinterpret_cast<char*>(&header), sizeof(header));
    return file.good() && header.magic == FrameRecordHeader::MAGIC;
}

bool SpatialFramePlayer::seek(uint64_t timestamp) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = std::lower_bound(frameTimestamps_.begin(), frameTimestamps_.end(), timestamp);
    if (it != frameTimestamps_.end()) {
        currentFrameIndex_ = std::distance(frameTimestamps_.begin(), it);
        return true;
    }
    
    return false;
}

bool SpatialFramePlayer::seekToFrame(uint64_t frameId) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (frameId < framePositions_.size()) {
        currentFrameIndex_ = frameId;
        return true;
    }
    
    return false;
}

bool SpatialFramePlayer::nextFrame(SpatialFrame& frame) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (currentFrameIndex_ >= framePositions_.size()) {
        return false;
    }
    
    return readFrameAt(currentFrameIndex_++, frame);
}

bool SpatialFramePlayer::readFrameAt(uint64_t index, SpatialFrame& frame) {
    if (index >= framePositions_.size()) {
        return false;
    }
    
    file_.seekg(framePositions_[index]);
    
    FrameRecordHeader header;
    if (!readHeader(file_, header)) {
        return false;
    }
    
    std::vector<uint8_t> buffer(header.payload_size);
    file_.read(reinterpret_cast<char*>(buffer.data()), header.payload_size);
    
    if (!verifyChecksum(header, buffer.data())) {
        return false;
    }
    
    // Parse protobuf
    spatial::SpatialFrame proto_frame;
    if (!proto_frame.ParseFromArray(buffer.data(), buffer.size())) {
        return false;
    }
    
    // Convert to SpatialFrame
    frame.clear();
    frame.timestamp = proto_frame.timestamp();
    frame.frame_id = proto_frame.frame_id();
    frame.processing_time_ms = proto_frame.processing_time_ms();
    frame.is_key_frame = proto_frame.is_key_frame();
    
    // Convert assets
    for (const auto& proto_asset : proto_frame.assets()) {
        AssetPosition asset;
        asset.asset_id = proto_asset.asset_id();
        asset.asset_name = proto_asset.asset_name();
        asset.tag_id = proto_asset.tag_id();
        asset.x = proto_asset.x();
        asset.y = proto_asset.y();
        asset.vx = proto_asset.vx();
        asset.vy = proto_asset.vy();
        asset.risk_score = proto_asset.risk_score();
        asset.state = proto_asset.state();
        asset.size = proto_asset.size();
        asset.top_action = proto_asset.top_action();
        asset.top_score = proto_asset.top_score();
        asset.predicted_x = proto_asset.predicted_x();
        asset.predicted_y = proto_asset.predicted_y();
        asset.prediction_confidence = proto_asset.prediction_confidence();
        
        for (const auto& pair : proto_asset.action_scores()) {
            asset.action_scores[pair.first] = pair.second;
        }
        
        frame.assets.push_back(asset);
    }
    
    // Convert heatmap
    for (const auto& proto_cell : proto_frame.heatmap()) {
        HeatCell cell;
        cell.x = proto_cell.x();
        cell.y = proto_cell.y();
        cell.value = proto_cell.value();
        frame.heatmap.push_back(cell);
    }
    
    // Convert congestion zones
    for (const auto& proto_zone : proto_frame.congestion()) {
        CongestionZone zone;
        zone.id = proto_zone.id();
        zone.name = proto_zone.name();
        zone.x = proto_zone.x();
        zone.y = proto_zone.y();
        zone.radius = proto_zone.radius();
        zone.density = proto_zone.density();
        zone.asset_count = proto_zone.asset_count();
        zone.level = proto_zone.level();
        frame.congestion.push_back(zone);
    }
    
    // Convert decisions
    for (const auto& proto_decision : proto_frame.decisions()) {
        DecisionPoint decision;
        decision.asset_id = proto_decision.asset_id();
        decision.action = proto_decision.action();
        decision.confidence = proto_decision.confidence();
        decision.expected_reward = proto_decision.expected_reward();
        decision.reason = proto_decision.reason();
        frame.decisions.push_back(decision);
    }
    
    // Convert predictions
    for (const auto& proto_prediction : proto_frame.predictions()) {
        PathPrediction prediction;
        prediction.asset_id = proto_prediction.asset_id();
        prediction.x = proto_prediction.x();
        prediction.y = proto_prediction.y();
        prediction.confidence = proto_prediction.confidence();
        prediction.timestamp = proto_prediction.timestamp();
        frame.predictions.push_back(prediction);
    }
    
    lastFrameHash_ = proto_frame.frame_hash();
    
    return true;
}

bool SpatialFramePlayer::verifyChecksum(const FrameRecordHeader& header, const uint8_t* data) {
    uint32_t crc = 0xFFFFFFFF;
    size_t size = header.payload_size;
    while (size--) {
        crc ^= *data++;
        for (int i = 0; i < 8; ++i) {
            crc = (crc >> 1) ^ (crc & 1 ? 0xEDB88320 : 0);
        }
    }
    return (~crc) == header.checksum;
}

void SpatialFramePlayer::setSpeed(double speed) {
    speed_ = std::max(0.1, std::min(16.0, speed));
}

double SpatialFramePlayer::getSpeed() const {
    return speed_;
}

void SpatialFramePlayer::play() {
    if (state_ == PlaybackState::PLAYING) {
        return;
    }
    
    state_ = PlaybackState::PLAYING;
    
    if (!running_) {
        running_ = true;
        playbackThread_ = std::thread(&SpatialFramePlayer::playbackLoop, this);
    } else {
        queueCondition_.notify_all();
    }
}

void SpatialFramePlayer::pause() {
    state_ = PlaybackState::PAUSED;
}

void SpatialFramePlayer::stop() {
    state_ = PlaybackState::STOPPED;
    currentFrameIndex_ = 0;
    
    {
        std::lock_guard<std::mutex> lock(mutex_);
        while (!frameQueue_.empty()) {
            frameQueue_.pop();
        }
    }
    
    queueCondition_.notify_all();
}

SpatialFramePlayer::PlaybackState SpatialFramePlayer::getState() const {
    return state_;
}

uint64_t SpatialFramePlayer::totalFrames() const {
    return framePositions_.size();
}

uint64_t SpatialFramePlayer::firstTimestamp() const {
    return frameTimestamps_.empty() ? 0 : frameTimestamps_.front();
}

uint64_t SpatialFramePlayer::lastTimestamp() const {
    return frameTimestamps_.empty() ? 0 : frameTimestamps_.back();
}

uint64_t SpatialFramePlayer::currentFrameId() const {
    return currentFrameIndex_;
}

uint64_t SpatialFramePlayer::currentTimestamp() const {
    if (currentFrameIndex_ < frameTimestamps_.size()) {
        return frameTimestamps_[currentFrameIndex_];
    }
    return 0;
}

uint64_t SpatialFramePlayer::getLastFrameHash() const {
    return lastFrameHash_;
}

void SpatialFramePlayer::playbackLoop() {
    const double baseInterval = 100.0; // 10 FPS = 100ms per frame
    
    while (running_) {
        {
            std::unique_lock<std::mutex> lock(mutex_);
            queueCondition_.wait(lock, [this] {
                return !running_ || 
                       state_ == PlaybackState::PLAYING || 
                       state_ == PlaybackState::PAUSED;
            });
            
            if (!running_) {
                break;
            }
            
            if (state_ == PlaybackState::PAUSED) {
                continue;
            }
            
            if (state_ == PlaybackState::STOPPED) {
                continue;
            }
            
            // Maintain ring buffer
            while (frameQueue_.size() >= MAX_QUEUE_SIZE) {
                frameQueue_.pop();
            }
        }
        
        // Read next frame
        SpatialFrame frame;
        if (!nextFrame(frame)) {
            // End of file
            state_ = PlaybackState::STOPPED;
            continue;
        }
        
        {
            std::lock_guard<std::mutex> lock(mutex_);
            frameQueue_.push(frame);
        }
        
        // Throttle based on speed
        double interval = baseInterval / speed_;
        std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(interval)));
    }
}