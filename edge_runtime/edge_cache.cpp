#include "edge_cache.h"

namespace edge_runtime {

EdgeCache& EdgeCache::instance() {
    static EdgeCache cache;
    return cache;
}

void EdgeCache::initialize(size_t max_size_bytes) {
    max_size_bytes_ = max_size_bytes;
    current_size_bytes_ = 0;
}

void EdgeCache::shutdown() {
    clear();
}

void EdgeCache::put(uint64_t key, const std::string& data) {
    evictIfNeeded();
    
    auto it = cache_.find(key);
    if (it != cache_.end()) {
        current_size_bytes_ -= it->second.data.size();
    }
    
    CacheEntry entry;
    entry.key = key;
    entry.data = data;
    entry.timestamp = 0;
    entry.access_count = 0;
    
    cache_[key] = entry;
    current_size_bytes_ += data.size();
}

bool EdgeCache::get(uint64_t key, std::string& data) {
    auto it = cache_.find(key);
    if (it == cache_.end()) {
        return false;
    }
    
    it->second.access_count++;
    it->second.timestamp = 0;
    data = it->second.data;
    return true;
}

bool EdgeCache::remove(uint64_t key) {
    auto it = cache_.find(key);
    if (it == cache_.end()) {
        return false;
    }
    
    current_size_bytes_ -= it->second.data.size();
    cache_.erase(it);
    return true;
}

void EdgeCache::clear() {
    cache_.clear();
    current_size_bytes_ = 0;
}

size_t EdgeCache::size() const {
    return cache_.size();
}

size_t EdgeCache::getMaxSize() const {
    return max_size_bytes_;
}

bool EdgeCache::isFull() const {
    return current_size_bytes_ >= max_size_bytes_;
}

void EdgeCache::setMaxSize(size_t max_size_bytes) {
    max_size_bytes_ = max_size_bytes;
    evictIfNeeded();
}

void EdgeCache::evictIfNeeded() {
    while (current_size_bytes_ >= max_size_bytes_ && !cache_.empty()) {
        uint64_t oldest_key = 0;
        uint64_t oldest_time = UINT64_MAX;
        
        for (const auto& pair : cache_) {
            if (pair.second.timestamp < oldest_time) {
                oldest_time = pair.second.timestamp;
                oldest_key = pair.first;
            }
        }
        
        remove(oldest_key);
    }
}

} // namespace edge_runtime