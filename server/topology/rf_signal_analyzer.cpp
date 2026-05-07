#include "rf_signal_analyzer.h"
#include <cmath>

void RFSignalAnalyzer::updateReaderPosition(uint64_t reader_id, float x, float y, float coverage_radius) {
    std::lock_guard<std::mutex> lock(mutex_);
    readers_[reader_id] = {x, y, coverage_radius, 0.0f, 0};
}

float RFSignalAnalyzer::calculateOverlap(uint64_t reader_a, uint64_t reader_b) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto a_it = readers_.find(reader_a);
    auto b_it = readers_.find(reader_b);
    
    if (a_it == readers_.end() || b_it == readers_.end()) return 0.0f;
    
    float dx = a_it->second.x - b_it->second.x;
    float dy = a_it->second.y - b_it->second.y;
    float distance = std::sqrt(dx * dx + dy * dy);
    
    float coverage_sum = a_it->second.coverage_radius + b_it->second.coverage_radius;
    
    if (distance >= coverage_sum) return 0.0f;
    
    return 1.0f - (distance / coverage_sum);
}

float RFSignalAnalyzer::calculateDistance(uint64_t reader_a, uint64_t reader_b) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto a_it = readers_.find(reader_a);
    auto b_it = readers_.find(reader_b);
    
    if (a_it == readers_.end() || b_it == readers_.end()) return 0.0f;
    
    float dx = a_it->second.x - b_it->second.x;
    float dy = a_it->second.y - b_it->second.y;
    
    return std::sqrt(dx * dx + dy * dy);
}

void RFSignalAnalyzer::updateSignalStrength(uint64_t reader_id, float rssi) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = readers_.find(reader_id);
    if (it == readers_.end()) return;
    
    ReaderInfo& info = it->second;
    info.avg_rssi = (info.avg_rssi * info.rssi_count + rssi) / (info.rssi_count + 1);
    info.rssi_count++;
}

float RFSignalAnalyzer::getAverageRSSI(uint64_t reader_id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = readers_.find(reader_id);
    return it != readers_.end() ? it->second.avg_rssi : 0.0f;
}