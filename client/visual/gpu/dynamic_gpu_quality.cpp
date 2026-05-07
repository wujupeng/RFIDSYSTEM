#include "dynamic_gpu_quality.h"

DynamicGPUQuality::DynamicGPUQuality()
    : asset_count_(0), current_level_(QualityLevel::HIGH) {
}

DynamicGPUQuality::~DynamicGPUQuality() {
    shutdown();
}

DynamicGPUQuality& DynamicGPUQuality::instance() {
    static DynamicGPUQuality instance;
    return instance;
}

void DynamicGPUQuality::initialize() {
    asset_count_ = 0;
    current_level_ = QualityLevel::HIGH;
}

void DynamicGPUQuality::shutdown() {
}

void DynamicGPUQuality::updateAssetCount(uint32_t count) {
    asset_count_ = count;
    updateQualityLevel();
}

DynamicGPUQuality::QualityLevel DynamicGPUQuality::getCurrentLevel() const {
    return current_level_;
}

bool DynamicGPUQuality::shouldRenderFullTrail() const {
    return current_level_ <= QualityLevel::MEDIUM;
}

bool DynamicGPUQuality::shouldRenderTrailGlow() const {
    return current_level_ <= QualityLevel::HIGH;
}

bool DynamicGPUQuality::shouldRenderHeatmapBlur() const {
    return current_level_ <= QualityLevel::MEDIUM;
}

float DynamicGPUQuality::getHeatmapResolutionScale() const {
    switch (current_level_) {
        case QualityLevel::HIGH:
            return 1.0f;
        case QualityLevel::MEDIUM:
            return 1.0f;
        case QualityLevel::LOW:
            return 0.75f;
        case QualityLevel::CRITICAL:
            return 0.5f;
        default:
            return 1.0f;
    }
}

float DynamicGPUQuality::getTrailLengthScale() const {
    switch (current_level_) {
        case QualityLevel::HIGH:
            return 1.0f;
        case QualityLevel::MEDIUM:
            return 0.7f;
        case QualityLevel::LOW:
            return 0.4f;
        case QualityLevel::CRITICAL:
            return 0.2f;
        default:
            return 1.0f;
    }
}

float DynamicGPUQuality::getOverlayDensityScale() const {
    switch (current_level_) {
        case QualityLevel::HIGH:
            return 1.0f;
        case QualityLevel::MEDIUM:
            return 0.8f;
        case QualityLevel::LOW:
            return 0.5f;
        case QualityLevel::CRITICAL:
            return 0.3f;
        default:
            return 1.0f;
    }
}

uint32_t DynamicGPUQuality::getMaxOverlayCount() const {
    switch (current_level_) {
        case QualityLevel::HIGH:
            return 100;
        case QualityLevel::MEDIUM:
            return 75;
        case QualityLevel::LOW:
            return 50;
        case QualityLevel::CRITICAL:
            return 25;
        default:
            return 100;
    }
}

uint32_t DynamicGPUQuality::getMaxTrailPoints() const {
    switch (current_level_) {
        case QualityLevel::HIGH:
            return 50000;
        case QualityLevel::MEDIUM:
            return 30000;
        case QualityLevel::LOW:
            return 15000;
        case QualityLevel::CRITICAL:
            return 5000;
        default:
            return 50000;
    }
}

void DynamicGPUQuality::updateQualityLevel() {
    uint32_t count = asset_count_;
    
    if (count >= CRITICAL_THRESHOLD) {
        current_level_ = QualityLevel::CRITICAL;
    } else if (count >= LOW_THRESHOLD) {
        current_level_ = QualityLevel::LOW;
    } else if (count >= MEDIUM_THRESHOLD) {
        current_level_ = QualityLevel::MEDIUM;
    } else {
        current_level_ = QualityLevel::HIGH;
    }
}