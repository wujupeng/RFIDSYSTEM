#include "spatial_frame_recorder.h"
#include "spatial_frame.h"
#include "protocol/spatial_frame.pb.h"
#include <cstring>
#include <cstdint>

SpatialFrameRecorder::SpatialFrameRecorder() 
    : total_frames_(0), total_bytes_(0), opened_(false) {
}

SpatialFrameRecorder::~SpatialFrameRecorder() {
    close();
}

bool SpatialFrameRecorder::open(const std::string& path) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (opened_) {
        close();
    }
    
    file_.open(path, std::ios::binary | std::ios::app);
    if (!file_.is_open()) {
        return false;
    }
    
    opened_ = true;
    return true;
}

void SpatialFrameRecorder::close() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (opened_) {
        flush();
        file_.close();
        opened_ = false;
    }
}

bool SpatialFrameRecorder::isOpen() const {
    return opened_;
}

void SpatialFrameRecorder::append(const SpatialFrame& frame) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!opened_) {
        return;
    }
    
    // Convert to protobuf
    spatial::SpatialFrame proto_frame;
    proto_frame.set_timestamp(frame.timestamp);
    proto_frame.set_frame_id(frame.frame_id);
    proto_frame.set_processing_time_ms(frame.processing_time_ms);
    proto_frame.set_is_key_frame(frame.is_key_frame);
    proto_frame.set_frame_hash(frame.hash());
    
    // Convert assets
    for (const auto& asset : frame.assets) {
        auto* proto_asset = proto_frame.add_assets();
        proto_asset->set_asset_id(asset.asset_id);
        proto_asset->set_asset_name(asset.asset_name);
        proto_asset->set_tag_id(asset.tag_id);
        proto_asset->set_x(asset.x);
        proto_asset->set_y(asset.y);
        proto_asset->set_vx(asset.vx);
        proto_asset->set_vy(asset.vy);
        proto_asset->set_risk_score(asset.risk_score);
        proto_asset->set_state(asset.state);
        proto_asset->set_size(asset.size);
        proto_asset->set_top_action(asset.top_action);
        proto_asset->set_top_score(asset.top_score);
        proto_asset->set_predicted_x(asset.predicted_x);
        proto_asset->set_predicted_y(asset.predicted_y);
        proto_asset->set_prediction_confidence(asset.prediction_confidence);
        
        for (const auto& pair : asset.action_scores) {
            (*proto_asset->mutable_action_scores())[pair.first] = pair.second;
        }
    }
    
    // Convert heatmap
    for (const auto& cell : frame.heatmap) {
        auto* proto_cell = proto_frame.add_heatmap();
        proto_cell->set_x(cell.x);
        proto_cell->set_y(cell.y);
        proto_cell->set_value(cell.value);
    }
    
    // Convert congestion zones
    for (const auto& zone : frame.congestion) {
        auto* proto_zone = proto_frame.add_congestion();
        proto_zone->set_id(zone.id);
        proto_zone->set_name(zone.name);
        proto_zone->set_x(zone.x);
        proto_zone->set_y(zone.y);
        proto_zone->set_radius(zone.radius);
        proto_zone->set_density(zone.density);
        proto_zone->set_asset_count(zone.asset_count);
        proto_zone->set_level(zone.level);
    }
    
    // Convert decisions
    for (const auto& decision : frame.decisions) {
        auto* proto_decision = proto_frame.add_decisions();
        proto_decision->set_asset_id(decision.asset_id);
        proto_decision->set_action(decision.action);
        proto_decision->set_confidence(decision.confidence);
        proto_decision->set_expected_reward(decision.expected_reward);
        proto_decision->set_reason(decision.reason);
    }
    
    // Convert predictions
    for (const auto& prediction : frame.predictions) {
        auto* proto_prediction = proto_frame.add_predictions();
        proto_prediction->set_asset_id(prediction.asset_id);
        proto_prediction->set_x(prediction.x);
        proto_prediction->set_y(prediction.y);
        proto_prediction->set_confidence(prediction.confidence);
        proto_prediction->set_timestamp(prediction.timestamp);
    }
    
    // Serialize to bytes
    std::string serialized = proto_frame.SerializeAsString();
    
    // Write header
    FrameRecordHeader header;
    header.magic = FrameRecordHeader::MAGIC;
    header.timestamp = frame.timestamp;
    header.payload_size = static_cast<uint32_t>(serialized.size());
    header.checksum = computeChecksum(reinterpret_cast<const uint8_t*>(serialized.data()), serialized.size());
    
    file_.write(reinterpret_cast<const char*>(&header), sizeof(header));
    file_.write(serialized.data(), serialized.size());
    
    total_frames_++;
    total_bytes_ += sizeof(header) + serialized.size();
}

void SpatialFrameRecorder::flush() {
    if (opened_) {
        file_.flush();
    }
}

uint64_t SpatialFrameRecorder::totalFrames() const {
    return total_frames_;
}

uint64_t SpatialFrameRecorder::totalBytes() const {
    return total_bytes_;
}

uint32_t SpatialFrameRecorder::computeChecksum(const uint8_t* data, size_t size) {
    // Simple CRC32-like checksum
    uint32_t crc = 0xFFFFFFFF;
    while (size--) {
        crc ^= *data++;
        for (int i = 0; i < 8; ++i) {
            crc = (crc >> 1) ^ (crc & 1 ? 0xEDB88320 : 0);
        }
    }
    return ~crc;
}