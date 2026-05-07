#pragma once

#include <cstdint>
#include <atomic>

class DynamicGPUQuality {
public:
    enum class QualityLevel {
        HIGH,
        MEDIUM,
        LOW,
        CRITICAL
    };
    
    static DynamicGPUQuality& instance();
    
    void initialize();
    void shutdown();
    
    void updateAssetCount(uint32_t count);
    
    QualityLevel getCurrentLevel() const;
    
    bool shouldRenderFullTrail() const;
    bool shouldRenderTrailGlow() const;
    bool shouldRenderHeatmapBlur() const;
    
    float getHeatmapResolutionScale() const;
    float getTrailLengthScale() const;
    float getOverlayDensityScale() const;
    
    uint32_t getMaxOverlayCount() const;
    uint32_t getMaxTrailPoints() const;
    
private:
    DynamicGPUQuality();
    ~DynamicGPUQuality();
    
    void updateQualityLevel();
    
    std::atomic<uint32_t> asset_count_;
    std::atomic<QualityLevel> current_level_;
    
    static constexpr uint32_t HIGH_THRESHOLD = 10000;
    static constexpr uint32_t MEDIUM_THRESHOLD = 30000;
    static constexpr uint32_t LOW_THRESHOLD = 50000;
    static constexpr uint32_t CRITICAL_THRESHOLD = 100000;
};