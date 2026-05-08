#pragma once

#include <cstdint>
#include <vector>

namespace ground_truth {

struct AlignmentTransform {
    float translation_x;
    float translation_y;
    float translation_z;
    float rotation_deg;
    float scaling;
};

struct AlignmentResult {
    bool success;
    AlignmentTransform transform;
    float alignment_error;
    float confidence;
};

class TruthAlignmentEngine {
public:
    static TruthAlignmentEngine& instance();
    
    AlignmentResult computeAlignment();
    
    void applyAlignment(float& x, float& y, float& z) const;
    
    void applyInverseAlignment(float& x, float& y, float& z) const;
    
    const AlignmentTransform& getCurrentTransform() const;
    
    void resetAlignment();
    
    bool isAligned() const;
    
    void updateAlignment();
    
private:
    TruthAlignmentEngine() = default;
    
    void solveLeastSquaresAlignment();
    
    AlignmentTransform current_transform_;
    bool aligned_ = false;
};

} // namespace ground_truth