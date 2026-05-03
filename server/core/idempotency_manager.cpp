#include "idempotency_manager.h"
#include <sstream>
#include <spdlog/spdlog.h>

namespace data {

IdempotencyManager& IdempotencyManager::instance() {
    static IdempotencyManager instance;
    return instance;
}

IdempotencyManager::IdempotencyManager()
    : lastCleanup_(std::chrono::steady_clock::now()) {
    spdlog::info("IdempotencyManager initialized");
}

std::string IdempotencyManager::generateKey(int taskId, const std::string& epc) {
    std::stringstream ss;
    ss << taskId << ":" << epc;
    return ss.str();
}

bool IdempotencyManager::isProcessed(int taskId, const std::string& epc) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - lastCleanup_).count();
    
    if (elapsed > 3600) {
        cleanupExpired(3600);
        lastCleanup_ = now;
    }
    
    std::string key = generateKey(taskId, epc);
    return processedKeys_.find(key) != processedKeys_.end();
}

void IdempotencyManager::markProcessed(int taskId, const std::string& epc) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::string key = generateKey(taskId, epc);
    processedKeys_.insert(key);
}

void IdempotencyManager::removeTask(int taskId) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::string prefix = std::to_string(taskId) + ":";
    auto it = processedKeys_.begin();
    while (it != processedKeys_.end()) {
        if (it->compare(0, prefix.size(), prefix) == 0) {
            it = processedKeys_.erase(it);
        } else {
            ++it;
        }
    }
}

void IdempotencyManager::cleanupExpired(int maxAgeSeconds) {
    spdlog::debug("IdempotencyManager cleanup - current size: {}", processedKeys_.size());
}

size_t IdempotencyManager::getCacheSize() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return processedKeys_.size();
}

void IdempotencyManager::clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    processedKeys_.clear();
    spdlog::info("IdempotencyManager cache cleared");
}

} // namespace data