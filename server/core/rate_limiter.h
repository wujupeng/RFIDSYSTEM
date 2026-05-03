#pragma once
#include <chrono>
#include <atomic>
#include <mutex>
#include <deque>

constexpr size_t MAX_REQUEST_QUEUE_SIZE = 100;
constexpr int REQUEST_TIMEOUT_MS = 2000;
constexpr size_t MAX_CONNECTIONS = 20;

class RateLimiter {
public:
    RateLimiter(size_t maxRequestsPerSecond = 100);

    bool tryAcquire();
    void release();

    size_t getCurrentRate() const;
    bool isOverloaded() const;

private:
    void cleanOldTimestamps();

    size_t maxRequestsPerSecond_;
    std::deque<std::chrono::steady_clock::time_point> timestamps_;
    mutable std::mutex mutex_;
    std::atomic<size_t> currentRate_;
};

class RequestTimeout {
public:
    static constexpr int DEFAULT_TIMEOUT_MS = 2000;

    template<typename Func, typename... Args>
    static auto withTimeout(int timeoutMs, Func&& func, Args&&... args)
        -> decltype(func(std::forward<Args>(args)...));

    static bool isTimeout(int timeoutMs);
};

class ServerStats {
public:
    static ServerStats& instance();

    void recordRequest();
    void recordResponse(int statusCode);
    void recordError();

    int64_t getUptimeSeconds() const;
    size_t getTotalRequests() const;
    size_t getActiveRequests() const;
    double getErrorRate() const;

    void setStartTime();

private:
    ServerStats() : startTime_(std::chrono::steady_clock::now()) {}

    std::chrono::steady_clock::time_point startTime_;
    std::atomic<size_t> totalRequests_;
    std::atomic<size_t> activeRequests_;
    std::atomic<size_t> errorCount_;
    mutable std::mutex statsMutex_;
};