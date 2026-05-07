#pragma once

#include "spatial_frame.h"
#include <string>
#include <vector>
#include <memory>

struct ValidationResult {
    bool valid;
    uint64_t frame_index;
    uint64_t expected_hash;
    uint64_t actual_hash;
    std::string mismatch_type;
    std::string message;
};

class ReplayValidator {
public:
    static ReplayValidator& instance();
    
    ValidationResult validateFrame(const SpatialFrame& realtimeFrame, const SpatialFrame& replayFrame);
    
    std::vector<ValidationResult> validateRecording(const std::string& recordingPath);
    
    uint64_t computeFrameHash(const SpatialFrame& frame) const;
    uint64_t computeOverlayHash(const SpatialFrame& frame) const;
    uint64_t computeGPUUploadHash(const SpatialFrame& frame) const;
    
    bool validateDecisionHash(const SpatialFrame& frame) const;
    
private:
    ReplayValidator();
    
    uint64_t hashCombine(uint64_t seed, uint64_t value) const;
    uint64_t hashFloat(float value) const;
};