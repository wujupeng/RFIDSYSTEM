#include "reader_health_model.h"
#include <thread>
#include <algorithm>

ReaderHealthModel::ReaderHealthModel()
    : computing_(false) {
}

ReaderHealthModel& ReaderHealthModel::instance() {
    static ReaderHealthModel instance;
    return instance;
}

void ReaderHealthModel::initialize() {
    std::lock_guard<std::mutex> lock(mutex_);
    readers_.clear();
    computing_ = false;
}

void ReaderHealthModel::shutdown() {
    computing_ = true;
    readers_.clear();
}

void ReaderHealthModel::updateReaderStats(uint32_t readerId, float missRate, float overlapRate, float coverageDensity) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = std::find_if(readers_.begin(), readers_.end(),
                          [readerId](const ReaderHealth& r) { return r.reader_id == readerId; });
    
    if (it != readers_.end()) {
        it->miss_rate = missRate;
        it->overlap_rate = overlapRate;
        it->coverage_density = coverageDensity;
    } else {
        ReaderHealth health;
        health.reader_id = readerId;
        health.miss_rate = missRate;
        health.overlap_rate = overlapRate;
        health.coverage_density = coverageDensity;
        readers_.push_back(health);
    }
}

ReaderHealth ReaderHealthModel::getReaderHealth(uint32_t readerId) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = std::find_if(readers_.begin(), readers_.end(),
                          [readerId](const ReaderHealth& r) { return r.reader_id == readerId; });
    
    if (it != readers_.end()) {
        return *it;
    }
    
    return ReaderHealth();
}

std::vector<ReaderHealth> ReaderHealthModel::getAllReaderHealths() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return readers_;
}

void ReaderHealthModel::computeOptimizations() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    for (auto& reader : readers_) {
        float healthScore = 1.0f;
        
        if (reader.miss_rate > MISS_RATE_THRESHOLD) {
            healthScore -= 0.3f * (reader.miss_rate - MISS_RATE_THRESHOLD) / MISS_RATE_THRESHOLD;
        }
        
        if (reader.overlap_rate > OVERLAP_THRESHOLD) {
            healthScore -= 0.2f * (reader.overlap_rate - OVERLAP_THRESHOLD) / OVERLAP_THRESHOLD;
        }
        
        if (reader.coverage_density < COVERAGE_LOW_THRESHOLD) {
            healthScore -= 0.3f * (COVERAGE_LOW_THRESHOLD - reader.coverage_density) / COVERAGE_LOW_THRESHOLD;
        } else if (reader.coverage_density > COVERAGE_HIGH_THRESHOLD) {
            healthScore -= 0.1f * (reader.coverage_density - COVERAGE_HIGH_THRESHOLD) / COVERAGE_HIGH_THRESHOLD;
        }
        
        healthScore = std::max(0.0f, std::min(1.0f, healthScore));
        reader.health_score = healthScore;
        
        reader.suggested_power_delta = 0.0f;
        reader.suggested_interval_delta = 0;
        reader.needs_reposition = false;
        
        if (reader.miss_rate > MISS_RATE_THRESHOLD) {
            reader.suggested_power_delta = 2.0f;
            reader.suggested_interval_delta = -20;
        }
        
        if (reader.overlap_rate > OVERLAP_THRESHOLD) {
            reader.suggested_power_delta = -1.0f;
        }
        
        if (reader.coverage_density < COVERAGE_LOW_THRESHOLD) {
            reader.needs_reposition = true;
            reader.suggested_power_delta = 3.0f;
        }
        
        if (reader.coverage_density > COVERAGE_HIGH_THRESHOLD) {
            reader.suggested_interval_delta = 30;
        }
    }
}

std::future<void> ReaderHealthModel::computeOptimizationsAsync() {
    return std::async(std::launch::async, [this]() {
        computeOptimizations();
    });
}