#include "runtime_failure_bus.h"
#include <algorithm>

RuntimeFailureBus::RuntimeFailureBus() : has_critical_(false) {
    for (int i = 0; i < 11; ++i) {
        failure_counts_[i] = 0;
    }
}

RuntimeFailureBus& RuntimeFailureBus::instance() {
    static RuntimeFailureBus instance;
    return instance;
}

void RuntimeFailureBus::initialize() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    recent_failures_.clear();
    has_critical_ = false;
    
    for (int i = 0; i < 11; ++i) {
        failure_counts_[i] = 0;
    }
}

void RuntimeFailureBus::shutdown() {
    std::lock_guard<std::mutex> lock(mutex_);
    recent_failures_.clear();
    callbacks_.clear();
}

void RuntimeFailureBus::publish(const FailureEvent& event) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    FailureEvent e = event;
    e.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    
    if (recent_failures_.size() >= MAX_RECENT_FAILURES) {
        recent_failures_.erase(recent_failures_.begin());
    }
    
    recent_failures_.push_back(e);
    
    int typeIndex = static_cast<int>(e.type);
    if (typeIndex >= 0 && typeIndex < 11) {
        failure_counts_[typeIndex]++;
    }
    
    if (e.severity >= CRITICAL_SEVERITY_THRESHOLD) {
        has_critical_ = true;
    }
    
    for (const auto& callback : callbacks_) {
        try {
            callback(e);
        } catch (...) {
        }
    }
}

void RuntimeFailureBus::acknowledge(FailureType type, uint64_t frameId) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    for (auto& failure : recent_failures_) {
        if (failure.type == type && failure.frame_id == frameId) {
            failure.acknowledged = true;
        }
    }
    
    bool hasUnackCritical = false;
    for (const auto& failure : recent_failures_) {
        if (!failure.acknowledged && failure.severity >= CRITICAL_SEVERITY_THRESHOLD) {
            hasUnackCritical = true;
            break;
        }
    }
    
    has_critical_ = hasUnackCritical;
}

std::vector<FailureEvent> RuntimeFailureBus::getRecent(int n) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::vector<FailureEvent> result;
    
    size_t start = n >= static_cast<int>(recent_failures_.size()) ? 
                   0 : recent_failures_.size() - n;
    
    for (size_t i = start; i < recent_failures_.size(); ++i) {
        result.push_back(recent_failures_[i]);
    }
    
    return result;
}

std::vector<FailureEvent> RuntimeFailureBus::getUnacknowledged() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::vector<FailureEvent> result;
    
    for (const auto& failure : recent_failures_) {
        if (!failure.acknowledged) {
            result.push_back(failure);
        }
    }
    
    return result;
}

bool RuntimeFailureBus::hasCriticalFailure() const {
    return has_critical_;
}

size_t RuntimeFailureBus::getFailureCount(FailureType type) const {
    int typeIndex = static_cast<int>(type);
    if (typeIndex >= 0 && typeIndex < 11) {
        return failure_counts_[typeIndex];
    }
    return 0;
}

size_t RuntimeFailureBus::getTotalFailureCount() const {
    size_t total = 0;
    for (int i = 0; i < 11; ++i) {
        total += failure_counts_[i];
    }
    return total;
}

void RuntimeFailureBus::clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    recent_failures_.clear();
    has_critical_ = false;
}

void RuntimeFailureBus::registerFailureCallback(std::function<void(const FailureEvent&)> callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    callbacks_.push_back(callback);
}