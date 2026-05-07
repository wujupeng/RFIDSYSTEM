#include "epc_processor.h"
#include "../core/logger.h"

namespace rfid {

EPCProcessor::EPCProcessor() {}

EPCProcessor& EPCProcessor::instance() {
    static EPCProcessor instance;
    return instance;
}

void EPCProcessor::setCallback(ProcessCallback callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    callback_ = callback;
}

void EPCProcessor::processEvents(const std::vector<EPCEvent>& events) {
    if (events.empty()) return;
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    for (const auto& event : events) {
        processedCount_++;
        
        if (isDuplicate(event.epc, event.timestamp_ms)) {
            filteredCount_++;
            continue;
        }
        
        auto key = event.epc;
        auto it = epcCache_.find(key);
        
        if (it == epcCache_.end()) {
            CacheEntry entry;
            entry.lastTimestamp = event.timestamp_ms;
            entry.bestRssi = event.rssi;
            epcCache_[key] = entry;
        } else {
            if (event.rssi > it->second.bestRssi) {
                it->second.bestRssi = event.rssi;
            }
            it->second.lastTimestamp = event.timestamp_ms;
        }
    }
    
    cleanupCache();
    
    if (callback_) {
        std::vector<EPCData> aggregated = ReaderManager::instance().aggregateEPCs(events);
        callback_(aggregated);
    }
}

void EPCProcessor::clearCache() {
    std::lock_guard<std::mutex> lock(mutex_);
    epcCache_.clear();
    processedCount_ = 0;
    filteredCount_ = 0;
    spdlog::info("EPC processor cache cleared");
}

void EPCProcessor::setDuplicateFilterWindow(int64_t windowMs) {
    std::lock_guard<std::mutex> lock(mutex_);
    duplicateFilterWindowMs_ = windowMs;
}

int64_t EPCProcessor::getDuplicateFilterWindow() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return duplicateFilterWindowMs_;
}

void EPCProcessor::setMaxCacheSize(size_t size) {
    std::lock_guard<std::mutex> lock(mutex_);
    maxCacheSize_ = size;
}

size_t EPCProcessor::getMaxCacheSize() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return maxCacheSize_;
}

size_t EPCProcessor::getCacheSize() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return epcCache_.size();
}

size_t EPCProcessor::getProcessedCount() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return processedCount_;
}

size_t EPCProcessor::getFilteredCount() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return filteredCount_;
}

bool EPCProcessor::isDuplicate(const std::string& epc, int64_t timestamp) {
    auto it = epcCache_.find(epc);
    if (it != epcCache_.end()) {
        int64_t diff = timestamp - it->second.lastTimestamp;
        if (diff < duplicateFilterWindowMs_) {
            return true;
        }
    }
    return false;
}

void EPCProcessor::cleanupCache() {
    int64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    
    int64_t expireTime = now - duplicateFilterWindowMs_;
    
    auto it = epcCache_.begin();
    while (it != epcCache_.end()) {
        if (it->second.lastTimestamp < expireTime) {
            it = epcCache_.erase(it);
        } else {
            ++it;
        }
    }
    
    while (epcCache_.size() > maxCacheSize_) {
        auto oldestIt = epcCache_.begin();
        int64_t oldestTime = oldestIt->second.lastTimestamp;
        
        for (auto iter = epcCache_.begin(); iter != epcCache_.end(); ++iter) {
            if (iter->second.lastTimestamp < oldestTime) {
                oldestTime = iter->second.lastTimestamp;
                oldestIt = iter;
            }
        }
        
        epcCache_.erase(oldestIt);
    }
}

} // namespace rfid