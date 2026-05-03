#pragma once
#include <string>
#include <unordered_set>
#include <mutex>
#include <chrono>
#include <memory>

namespace data {

class IdempotencyManager {
public:
    static IdempotencyManager& instance();
    
    bool isProcessed(int taskId, const std::string& epc);
    
    void markProcessed(int taskId, const std::string& epc);
    
    void removeTask(int taskId);
    
    void cleanupExpired(int maxAgeSeconds = 3600);
    
    size_t getCacheSize() const;
    
    void clear();
    
private:
    IdempotencyManager();
    ~IdempotencyManager() = default;
    IdempotencyManager(const IdempotencyManager&) = delete;
    IdempotencyManager& operator=(const IdempotencyManager&) = delete;
    
    std::string generateKey(int taskId, const std::string& epc);
    
    std::unordered_set<std::string> processedKeys_;
    mutable std::mutex mutex_;
    
    std::chrono::steady_clock::time_point lastCleanup_;
};

} // namespace data