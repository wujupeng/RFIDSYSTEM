#include "rate_limiter.h"
#include "logger.h"
#include <thread>
#include <chrono>

RateLimiter::RateLimiter(size_t maxRequestsPerSecond)
    : maxRequestsPerSecond_(maxRequestsPerSecond), currentRate_(0) {}

bool RateLimiter::tryAcquire() {
    std::lock_guard<std::mutex> lock(mutex_);

    cleanOldTimestamps();

    if (timestamps_.size() >= maxRequestsPerSecond_) {
        spdlog::warn("Rate limit exceeded: {} requests in queue", timestamps_.size());
        return false;
    }

    timestamps_.push_back(std::chrono::steady_clock::now());
    currentRate_ = timestamps_.size();
    return true;
}

void RateLimiter::release() {
    std::lock_guard<std::mutex> lock(mutex_);

    if (!timestamps_.empty()) {
        timestamps_.pop_front();
        currentRate_ = timestamps_.size();
    }
}

size_t RateLimiter::getCurrentRate() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return timestamps_.size();
}

bool RateLimiter::isOverloaded() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return timestamps_.size() >= maxRequestsPerSecond_ * 0.8;
}

void RateLimiter::cleanOldTimestamps() {
    auto now = std::chrono::steady_clock::now();
    auto oneSecondAgo = now - std::chrono::seconds(1);

    while (!timestamps_.empty() && timestamps_.front() < oneSecondAgo) {
        timestamps_.pop_front();
    }
}

template<typename Func, typename... Args>
auto RequestTimeout::withTimeout(int timeoutMs, Func&& func, Args&&... args)
    -> decltype(func(std::forward<Args>(args)...)) {
    using ReturnType = decltype(func(std::forward<Args>(args)...));

    if (timeoutMs <= 0) {
        timeoutMs = DEFAULT_TIMEOUT_MS;
    }

    std::promise<ReturnType> promise;
    std::future<ReturnType> future = promise.get_future();

    std::thread worker([&]() {
        try {
            promise.set_value(func(std::forward<Args>(args)...));
        } catch (...) {
            promise.set_exception(std::current_exception());
        }
    });

    auto status = future.wait_for(std::chrono::milliseconds(timeoutMs));

    if (status == std::future_status::timeout) {
        worker.detach();
        spdlog::error("Request timeout after {}ms", timeoutMs);
        throw std::runtime_error("Request timeout");
    }

    ReturnType result = future.get();
    worker.join();
    return result;
}

ServerStats& ServerStats::instance() {
    static ServerStats instance;
    return instance;
}

void ServerStats::recordRequest() {
    totalRequests_++;
    activeRequests_++;
}

void ServerStats::recordResponse(int statusCode) {
    activeRequests_--;
    if (statusCode >= 400) {
        errorCount_++;
    }
}

void ServerStats::recordError() {
    errorCount_++;
}

int64_t ServerStats::getUptimeSeconds() const {
    auto now = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::seconds>(now - startTime_).count();
}

size_t ServerStats::getTotalRequests() const {
    return totalRequests_;
}

size_t ServerStats::getActiveRequests() const {
    return activeRequests_;
}

double ServerStats::getErrorRate() const {
    size_t total = totalRequests_;
    if (total == 0) return 0.0;
    return static_cast<double>(errorCount_) / static_cast<double>(total);
}

void ServerStats::setStartTime() {
    startTime_ = std::chrono::steady_clock::now();
}