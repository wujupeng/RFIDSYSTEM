#pragma once

#include "topology_types.h"
#include <unordered_map>
#include <mutex>

class RFSignalAnalyzer {
public:
    void updateReaderPosition(uint64_t reader_id, float x, float y, float coverage_radius);
    
    float calculateOverlap(uint64_t reader_a, uint64_t reader_b) const;
    
    float calculateDistance(uint64_t reader_a, uint64_t reader_b) const;
    
    void updateSignalStrength(uint64_t reader_id, float rssi);
    
    float getAverageRSSI(uint64_t reader_id) const;
    
private:
    struct ReaderInfo {
        float x, y;
        float coverage_radius;
        float avg_rssi;
        int rssi_count;
    };
    
    std::unordered_map<uint64_t, ReaderInfo> readers_;
    
    mutable std::mutex mutex_;
};