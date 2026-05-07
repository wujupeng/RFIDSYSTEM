#pragma once

#include <cstdint>
#include <mutex>
#include <atomic>
#include <vector>
#include <chrono>

struct AIFrameBinding {
    uint64_t frame_id;
    uint64_t ai_result_id;
    float latency_ms;
    uint64_t timestamp;
    bool is_bound;
    
    AIFrameBinding() 
        : frame_id(0), ai_result_id(0), latency_ms(0.0f), 
          timestamp(0), is_bound(false) {}
};

class AIFrameLocking {
public:
    static AIFrameLocking& instance();
    
    void initialize();
    void shutdown();
    
    uint64_t acquireFrameLock(uint64_t frameId);
    
    bool bindAIResult(uint64_t frameId, uint64_t aiResultId);
    
    bool isFrameLocked(uint64_t frameId) const;
    
    AIFrameBinding getBinding(uint64_t frameId) const;
    
    void releaseFrameLock(uint64_t frameId);
    
    void cleanupOldBindings(uint64_t maxAgeMs);
    
    size_t getActiveBindingCount() const;
    
    float getAverageLatency() const;
    
private:
    AIFrameLocking();
    
    struct FrameLock {
        uint64_t frame_id;
        uint64_t lock_id;
        std::chrono::steady_clock::time_point lock_time;
        AIFrameBinding binding;
    };
    
    std::vector<FrameLock> locks_;
    mutable std::mutex mutex_;
    std::atomic<uint64_t> next_lock_id_;
};