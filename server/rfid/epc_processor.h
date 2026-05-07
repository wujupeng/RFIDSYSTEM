#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <chrono>
#include <functional>
#include "reader.h"

namespace rfid {

class EPCProcessor {
public:
    using ProcessCallback = std::function<void(const std::vector<EPCData>& aggregated)>;

    static EPCProcessor& instance();

    void setCallback(ProcessCallback callback);
    void processEvents(const std::vector<EPCEvent>& events);
    void clearCache();

    void setDuplicateFilterWindow(int64_t windowMs);
    int64_t getDuplicateFilterWindow() const;

    void setMaxCacheSize(size_t size);
    size_t getMaxCacheSize() const;

    size_t getCacheSize() const;
    size_t getProcessedCount() const;
    size_t getFilteredCount() const;

private:
    EPCProcessor();
    EPCProcessor(const EPCProcessor&) = delete;
    EPCProcessor& operator=(const EPCProcessor&) = delete;

    void onEvents(const std::vector<EPCEvent>& events);
    bool isDuplicate(const std::string& epc, int64_t timestamp);
    void cleanupCache();

    ProcessCallback callback_;
    
    int64_t duplicateFilterWindowMs_ = DUPLICATE_FILTER_WINDOW_MS;
    size_t maxCacheSize_ = MAX_BUFFER_SIZE;
    
    struct CacheEntry {
        int64_t lastTimestamp;
        int bestRssi;
    };
    
    std::unordered_map<std::string, CacheEntry> epcCache_;
    mutable std::mutex mutex_;
    
    size_t processedCount_ = 0;
    size_t filteredCount_ = 0;
};

} // namespace rfid