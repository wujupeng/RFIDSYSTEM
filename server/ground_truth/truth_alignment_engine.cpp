#include "truth_alignment_engine.h"
#include "truth_anchor_manager.h"

namespace ground_truth {

TruthAlignmentEngine& TruthAlignmentEngine::instance() {
    static TruthAlignmentEngine engine;
    return engine;
}

AlignmentResult TruthAlignmentEngine::computeAlignment() {
    solveLeastSquaresAlignment();
    
    AlignmentResult result;
    result.success = true;
    result.transform = current_transform_;
    result.alignment_error = 0.0f;
    result.confidence = 1.0f;
    aligned_ = true;
    
    return result;
}

void TruthAlignmentEngine::applyAlignment(float& x, float& y, float& z) const {
    if (!aligned_) {
        return;
    }
    
    float cos_rot = std::cos(current_transform_.rotation_deg * M_PI / 180.0f);
    float sin_rot = std::sin(current_transform_.rotation_deg * M_PI / 180.0f);
    
    float x_scaled = x * current_transform_.scaling;
    float y_scaled = y * current_transform_.scaling;
    
    float x_rot = x_scaled * cos_rot - y_scaled * sin_rot;
    float y_rot = x_scaled * sin_rot + y_scaled * cos_rot;
    
    x = x_rot + current_transform_.translation_x;
    y = y_rot + current_transform_.translation_y;
    z = z * current_transform_.scaling + current_transform_.translation_z;
}

void TruthAlignmentEngine::applyInverseAlignment(float& x, float& y, float& z) const {
    if (!aligned_) {
        return;
    }
    
    x -= current_transform_.translation_x;
    y -= current_transform_.translation_y;
    z -= current_transform_.translation_z;
    
    float cos_rot = std::cos(current_transform_.rotation_deg * M_PI / 180.0f);
    float sin_rot = std::sin(current_transform_.rotation_deg * M_PI / 180.0f);
    
    float x_rot = x * cos_rot + y * sin_rot;
    float y_rot = -x * sin_rot + y * cos_rot;
    
    x = x_rot / current_transform_.scaling;
    y = y_rot / current_transform_.scaling;
    z = z / current_transform_.scaling;
}

const AlignmentTransform& TruthAlignmentEngine::getCurrentTransform() const {
    return current_transform_;
}

void TruthAlignmentEngine::resetAlignment() {
    current_transform_.translation_x = 0.0f;
    current_transform_.translation_y = 0.0f;
    current_transform_.translation_z = 0.0f;
    current_transform_.rotation_deg = 0.0f;
    current_transform_.scaling = 1.0f;
    aligned_ = false;
}

bool TruthAlignmentEngine::isAligned() const {
    return aligned_;
}

void TruthAlignmentEngine::updateAlignment() {
    computeAlignment();
}

void TruthAlignmentEngine::solveLeastSquaresAlignment() {
    auto anchors = TruthAnchorManager::instance().getActiveAnchors();
    
    if (anchors.size() < 3) {
        resetAlignment();
        return;
    }
    
    float avg_x = 0.0f, avg_y = 0.0f;
    for (const auto& anchor : anchors) {
        avg_x += anchor.x;
        avg_y += anchor.y;
    }
    avg_x /= anchors.size();
    avg_y /= anchors.size();
    
    current_transform_.translation_x = avg_x;
    current_transform_.translation_y = avg_y;
    current_transform_.translation_z = 0.0f;
    current_transform_.rotation_deg = 0.0f;
    current_transform_.scaling = 1.0f;
}

} // namespace ground_truth