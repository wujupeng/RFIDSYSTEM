#include "ai_frame_locking.h"
#include <algorithm>

AIFrameLocking::AIFrameLocking()
    : next_lock_id_(1) {
}

AIFrameLocking& AIFrameLocking::instance() {
    static AIFrameLocking instance;
    return instance;
}

void AIFrameLocking::initialize() {
    next_lock_id_ = 1;
}

void AIFrameLocking::shutdown() {
    std::lock_guard<std::mutex> lock(mutex_);
    locks_.clear();
}

uint64_t AIFrameLocking::acquireFrameLock(uint64_t frameId) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    uint64_t lockId = next_lock_id_++;
    
    FrameLock fl;
    fl.frame_id = frameId;
    fl.lock_id = lockId;
    fl.lock_time = std::chrono::steady_clock::now();
    fl.binding.frame_id = frameId;
    fl.binding.is_bound = false;
    
    locks_.push_back(fl);
    
    return lockId;
}

bool AIFrameLocking::bindAIResult(uint64_t frameId, uint64_t aiResultId) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = std::find_if(locks_.begin(), locks_.end(),
                          [frameId](const FrameLock& fl) { return fl.frame_id == frameId; });
    
    if (it != locks_.end()) {
        auto now = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(now - it->lock_time);
        
        it->binding.ai_result_id = aiResultId;
        it->binding.latency_ms = static_cast<float>(duration.count()) / 1000.0f;
        it->binding.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        it->binding.is_bound = true;
        
        return true;
    }
    
    return false;
}

bool AIFrameLocking::isFrameLocked(uint64_t frameId) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = std::find_if(locks_.begin(), locks_.end(),
                          [frameId](const FrameLock& fl) { return fl.frame_id == frameId; });
    
    return it != locks_.end();
}

AIFrameBinding AIFrameLocking::getBinding(uint64_t frameId) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = std::find_if(locks_.begin(), locks_.end(),
                          [frameId](const FrameLock& fl) { return fl.frame_id == frameId; });
    
    if (it != locks_.end()) {
        return it->binding;
    }
    
    return AIFrameBinding();
}

void AIFrameLocking::releaseFrameLock(uint64_t frameId) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = std::remove_if(locks_.begin(), locks_.end(),
                            [frameId](const FrameLock& fl) { return fl.frame_id == frameId; });
    
    locks_.erase(it, locks_.end());
}

void AIFrameLocking::cleanupOldBindings(uint64_t maxAgeMs) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto now = std::chrono::steady_clock::now();
    
    auto it = std::remove_if(locks_.begin(), locks_.end(),
                            [now, maxAgeMs](const FrameLock& fl) {
                                auto age = std::chrono::duration_cast<std::chrono::milliseconds>(now - fl.lock_time);
                                return age.count() > maxAgeMs;
                            });
    
    locks_.erase(it, locks_.end());
}

size_t AIFrameLocking::getActiveBindingCount() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    return std::count_if(locks_.begin(), locks_.end(),
                        [](const FrameLock& fl) { return fl.binding.is_bound; });
}

float AIFrameLocking::getAverageLatency() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    float sum = 0.0f;
    size_t count = 0;
    
    for (const auto& fl : locks_) {
        if (fl.binding.is_bound) {
            sum += fl.binding.latency_ms;
            count++;
        }
    }
    
    return count > 0 ? sum / count : 0.0f;
}