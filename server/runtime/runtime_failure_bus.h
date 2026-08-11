#pragma once

#include <string>
#include <mutex>
#include <vector>
#include <functional>
#include <atomic>
#include <optional>

enum class FailureType {
    FRAME_HASH_MISMATCH,
    GPU_OVERRUN,
    AI_CONVERGENCE_COLLAPSE,
    REPLAY_DESYNC,
    FACTORY_DESYNC,
    POLICY_CORRUPTION,
    MEMORY_LEAK_DETECTED,
    NETWORK_TIMEOUT,
    SPATIAL_LOCALIZATION_DRIFT,
    UNKNOWN
};

struct FailureEvent {
    FailureType type;
    uint64_t frame_id;
    uint64_t timestamp;
    std::string message;
    double severity;
    bool acknowledged;
    std::optional<std::string> tag_id;
    std::optional<double> error_value;
    std::optional<double> threshold;
    
    FailureEvent()
        : type(FailureType::UNKNOWN), frame_id(0), timestamp(0),
          severity(0.0), acknowledged(false) {}
};

class RuntimeFailureBus {
public:
    static RuntimeFailureBus& instance();
    
    void initialize();
    void shutdown();
    
    void publish(const FailureEvent& event);
    
    void acknowledge(FailureType type, uint64_t frameId);
    
    std::vector<FailureEvent> getRecent(int n);
    
    std::vector<FailureEvent> getUnacknowledged();
    
    bool hasCriticalFailure() const;
    
    size_t getFailureCount(FailureType type) const;
    
    size_t getTotalFailureCount() const;
    
    void clear();
    
    void registerFailureCallback(std::function<void(const FailureEvent&)> callback);
    
private:
    RuntimeFailureBus();
    
    std::vector<FailureEvent> recent_failures_;
    std::vector<std::function<void(const FailureEvent&)>> callbacks_;
    
    mutable std::mutex mutex_;
    
    std::atomic<size_t> failure_counts_[11];
    std::atomic<bool> has_critical_;
    
    static constexpr size_t MAX_RECENT_FAILURES = 500;
    static constexpr double CRITICAL_SEVERITY_THRESHOLD = 0.8;
};