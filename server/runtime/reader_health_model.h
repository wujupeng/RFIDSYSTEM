#pragma once

#include <cstdint>
#include <vector>
#include <mutex>
#include <atomic>
#include <future>

struct ReaderHealth {
    uint32_t reader_id;
    float miss_rate;
    float overlap_rate;
    float coverage_density;
    int32_t power_level;
    int32_t scan_interval_ms;
    
    float health_score;
    float suggested_power_delta;
    int32_t suggested_interval_delta;
    bool needs_reposition;
    
    ReaderHealth() 
        : reader_id(0), miss_rate(0.0f), overlap_rate(0.0f), coverage_density(0.0f),
          power_level(0), scan_interval_ms(100), health_score(1.0f),
          suggested_power_delta(0.0f), suggested_interval_delta(0), needs_reposition(false) {}
};

class ReaderHealthModel {
public:
    static ReaderHealthModel& instance();
    
    void initialize();
    void shutdown();
    
    void updateReaderStats(uint32_t readerId, float missRate, float overlapRate, float coverageDensity);
    
    ReaderHealth getReaderHealth(uint32_t readerId) const;
    std::vector<ReaderHealth> getAllReaderHealths() const;
    
    void computeOptimizations();
    
    std::future<void> computeOptimizationsAsync();
    
private:
    ReaderHealthModel();
    
    std::vector<ReaderHealth> readers_;
    
    mutable std::mutex mutex_;
    std::atomic<bool> computing_;
    
    static constexpr float MISS_RATE_THRESHOLD = 0.15f;
    static constexpr float OVERLAP_THRESHOLD = 0.4f;
    static constexpr float COVERAGE_LOW_THRESHOLD = 0.3f;
    static constexpr float COVERAGE_HIGH_THRESHOLD = 0.8f;
};