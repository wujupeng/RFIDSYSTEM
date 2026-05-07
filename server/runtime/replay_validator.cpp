#include "replay_validator.h"
#include <algorithm>
#include <cmath>

ReplayValidator::ReplayValidator() {
}

ReplayValidator& ReplayValidator::instance() {
    static ReplayValidator instance;
    return instance;
}

ValidationResult ReplayValidator::validateFrame(const SpatialFrame& realtimeFrame, const SpatialFrame& replayFrame) {
    ValidationResult result;
    result.valid = true;
    result.frame_index = realtimeFrame.frame_id;
    
    uint64_t realtimeHash = computeFrameHash(realtimeFrame);
    uint64_t replayHash = computeFrameHash(replayFrame);
    
    result.expected_hash = replayHash;
    result.actual_hash = realtimeHash;
    
    if (realtimeHash != replayHash) {
        result.valid = false;
        result.mismatch_type = "FRAME_HASH";
        result.message = "Frame hash mismatch at frame " + std::to_string(realtimeFrame.frame_id);
        return result;
    }
    
    uint64_t realtimeOverlayHash = computeOverlayHash(realtimeFrame);
    uint64_t replayOverlayHash = computeOverlayHash(replayFrame);
    
    if (realtimeOverlayHash != replayOverlayHash) {
        result.valid = false;
        result.mismatch_type = "OVERLAY_HASH";
        result.message = "Overlay hash mismatch at frame " + std::to_string(realtimeFrame.frame_id);
        return result;
    }
    
    if (!validateDecisionHash(realtimeFrame)) {
        result.valid = false;
        result.mismatch_type = "DECISION_HASH";
        result.message = "Decision hash validation failed at frame " + std::to_string(realtimeFrame.frame_id);
        return result;
    }
    
    result.message = "Validation passed";
    return result;
}

std::vector<ValidationResult> ReplayValidator::validateRecording(const std::string& recordingPath) {
    std::vector<ValidationResult> results;
    return results;
}

uint64_t ReplayValidator::computeFrameHash(const SpatialFrame& frame) const {
    uint64_t result = 17;
    
    hashCombine(result, frame.frame_id);
    hashCombine(result, frame.timestamp);
    
    for (const auto& asset : frame.assets) {
        hashCombine(result, asset.asset_id);
        hashCombine(result, hashFloat(static_cast<float>(asset.x)));
        hashCombine(result, hashFloat(static_cast<float>(asset.y)));
    }
    
    for (const auto& overlay : frame.overlays) {
        hashCombine(result, overlay.asset_id);
        hashCombine(result, overlay.action);
        hashCombine(result, hashFloat(overlay.confidence));
    }
    
    return result;
}

uint64_t ReplayValidator::computeOverlayHash(const SpatialFrame& frame) const {
    uint64_t result = 37;
    
    for (const auto& overlay : frame.overlays) {
        hashCombine(result, overlay.asset_id);
        hashCombine(result, overlay.action);
        hashCombine(result, hashFloat(overlay.x));
        hashCombine(result, hashFloat(overlay.y));
        hashCombine(result, hashFloat(overlay.confidence));
        hashCombine(result, hashFloat(overlay.missing_risk));
        hashCombine(result, hashFloat(overlay.abnormal_risk));
        hashCombine(result, hashFloat(overlay.inactivity_risk));
        hashCombine(result, overlay.timestamp);
    }
    
    return result;
}

uint64_t ReplayValidator::computeGPUUploadHash(const SpatialFrame& frame) const {
    uint64_t result = 59;
    
    result = hashCombine(result, static_cast<uint64_t>(frame.assets.size()));
    result = hashCombine(result, static_cast<uint64_t>(frame.overlays.size()));
    result = hashCombine(result, static_cast<uint64_t>(frame.heatmap.size()));
    
    return result;
}

bool ReplayValidator::validateDecisionHash(const SpatialFrame& frame) const {
    uint64_t computed = frame.hash();
    return computed == frame.decision_hash;
}

uint64_t ReplayValidator::hashCombine(uint64_t seed, uint64_t value) const {
    seed ^= value + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    return seed;
}

uint64_t ReplayValidator::hashFloat(float value) const {
    uint64_t result = 0;
    memcpy(&result, &value, sizeof(float));
    return result;
}