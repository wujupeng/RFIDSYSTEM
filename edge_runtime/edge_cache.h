#pragma once

#include <cstdint>
#include <vector>
#include <map>
#include <string>

namespace edge_runtime {

struct CacheEntry {
    uint64_t key;
    std::string data;
    uint64_t timestamp;
    uint64_t access_count;
};

class EdgeCache {
public:
    static EdgeCache& instance();
    
    void initialize(size_t max_size_bytes);
    
    void shutdown();
    
    void put(uint64_t key, const std::string& data);
    
    bool get(uint64_t key, std::string& data);
    
    bool remove(uint64_t key);
    
    void clear();
    
    size_t size() const;
    
    size_t getMaxSize() const;
    
    bool isFull() const;
    
    void setMaxSize(size_t max_size_bytes);
    
private:
    EdgeCache() = default;
    
    void evictIfNeeded();
    
    std::map<uint64_t, CacheEntry> cache_;
    size_t max_size_bytes_ = 1024 * 1024 * 100;
    size_t current_size_bytes_ = 0;
};

} // namespace edge_runtime