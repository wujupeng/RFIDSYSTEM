#include "metrics.h"
#include "logger.h"
#include <algorithm>
#include <numeric>
#include <cmath>

Metrics& Metrics::instance() {
    static Metrics instance;
    return instance;
}

Metrics::Metrics()
    : startTime_(std::chrono::steady_clock::now())
    , lastEPCCountTime_(startTime_) {
    spdlog::info("Metrics system initialized");
}

void Metrics::incEPCProcessed(int count) {
    epcProcessed_.fetch_add(count, std::memory_order_relaxed);
    checkThresholds();
}

void Metrics::incEPCDropped(int count) {
    epcDropped_.fetch_add(count, std::memory_order_relaxed);
}

void Metrics::recordDBLatency(double ms) {
    std::lock_guard<std::mutex> lock(latencyMutex_);
    dbLatencies_.push_back(ms);
    if (dbLatencies_.size() > 1000) {
        dbLatencies_.erase(dbLatencies_.begin());
    }

    if (ms > DB_LATENCY_THRESHOLD_MS) {
        spdlog::warn("High DB latency detected: {:.2f}ms (threshold: {:.2f}ms)", ms, DB_LATENCY_THRESHOLD_MS);
    }
}

void Metrics::recordRequestLatency(double ms) {
    std::lock_guard<std::mutex> lock(latencyMutex_);
    requestLatencies_.push_back(ms);
    if (requestLatencies_.size() > 1000) {
        requestLatencies_.erase(requestLatencies_.begin());
    }
}

void Metrics::incRequestSuccess() {
    requestsSuccess_.fetch_add(1, std::memory_order_relaxed);
}

void Metrics::incRequestFailed() {
    requestsFailed_.fetch_add(1, std::memory_order_relaxed);
    checkThresholds();
}

void Metrics::setActiveReaders(int count) {
    int prev = activeReaders_.exchange(count);
    if (count == 0 && prev > 0) {
        if (alertCallback_) {
            alertCallback_("READER_DISCONNECT", "All readers disconnected");
        }
        spdlog::error("ALERT: All readers disconnected");
    }
}

void Metrics::setActiveTasks(int count) {
    activeTasks_.store(count, std::memory_order_relaxed);
}

void Metrics::recordTaskCompleted(const std::string& taskType, double durationMs) {
    spdlog::info("Task completed: type={}, duration={:.2f}ms", taskType, durationMs);
}

double Metrics::getEPCPerSecond() {
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - lastEPCCountTime_).count();

    if (elapsed < 1) {
        return 0.0;
    }

    int64_t current = epcProcessed_.load(std::memory_order_relaxed);
    int64_t diff = current - lastEPCProcessed_;

    lastEPCProcessed_ = current;
    lastEPCCountTime_ = now;

    return static_cast<double>(diff) / elapsed;
}

double Metrics::getDBLatencyP99() {
    std::lock_guard<std::mutex> lock(latencyMutex_);
    if (dbLatencies_.empty()) {
        return 0.0;
    }

    std::vector<double> sorted = dbLatencies_;
    std::sort(sorted.begin(), sorted.end());
    size_t idx = static_cast<size_t>(std::ceil(sorted.size() * 0.99)) - 1;
    return sorted[idx];
}

double Metrics::getRequestSuccessRate() {
    int64_t success = requestsSuccess_.load(std::memory_order_relaxed);
    int64_t failed = requestsFailed_.load(std::memory_order_relaxed);
    int64_t total = success + failed;

    if (total == 0) {
        return 1.0;
    }

    return static_cast<double>(success) / total;
}

double Metrics::getErrorRate() {
    int64_t success = requestsSuccess_.load(std::memory_order_relaxed);
    int64_t failed = requestsFailed_.load(std::memory_order_relaxed);
    int64_t total = success + failed;

    if (total == 0) {
        return 0.0;
    }

    return static_cast<double>(failed) / total;
}

int Metrics::getActiveReaders() {
    return activeReaders_.load(std::memory_order_relaxed);
}

int Metrics::getActiveTasks() {
    return activeTasks_.load(std::memory_order_relaxed);
}

std::unordered_map<std::string, std::string> Metrics::getAllMetrics() {
    std::unordered_map<std::string, std::string> result;

    auto now = std::chrono::steady_clock::now();
    auto uptime = std::chrono::duration_cast<std::chrono::seconds>(now - startTime_).count();

    result["uptime_seconds"] = std::to_string(uptime);
    result["epc_processed_total"] = std::to_string(epcProcessed_.load());
    result["epc_dropped_total"] = std::to_string(epcDropped_.load());
    result["epc_per_second"] = std::to_string(getEPCPerSecond());
    result["db_latency_p99_ms"] = std::to_string(getDBLatencyP99());
    result["request_success_rate"] = std::to_string(getRequestSuccessRate());
    result["error_rate"] = std::to_string(getErrorRate());
    result["active_readers"] = std::to_string(getActiveReaders());
    result["active_tasks"] = std::to_string(getActiveTasks());
    result["requests_success"] = std::to_string(requestsSuccess_.load());
    result["requests_failed"] = std::to_string(requestsFailed_.load());

    return result;
}

void Metrics::reset() {
    epcProcessed_.store(0);
    epcDropped_.store(0);
    requestsSuccess_.store(0);
    requestsFailed_.store(0);
    activeReaders_.store(0);
    activeTasks_.store(0);

    std::lock_guard<std::mutex> lock(latencyMutex_);
    dbLatencies_.clear();
    requestLatencies_.clear();

    startTime_ = std::chrono::steady_clock::now();
    lastEPCCountTime_ = startTime_;
    lastEPCProcessed_ = 0;

    spdlog::info("Metrics reset");
}

void Metrics::setAlertCallback(AlertCallback callback) {
    alertCallback_ = callback;
}

void Metrics::checkThresholds() {
    double errorRate = getErrorRate();
    if (errorRate > ERROR_RATE_THRESHOLD) {
        if (alertCallback_) {
            alertCallback_("HIGH_ERROR_RATE",
                "Error rate exceeded threshold: " + std::to_string(errorRate * 100) + "%");
        }
        spdlog::error("ALERT: Error rate {:.2f}% exceeded threshold {:.2f}%",
                     errorRate * 100, ERROR_RATE_THRESHOLD * 100);
    }
}