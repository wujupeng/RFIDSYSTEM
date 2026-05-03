#pragma once

#include <atomic>
#include <chrono>
#include <mutex>
#include <string>
#include <vector>
#include <functional>
#include <unordered_map>

class Metrics {
public:
    static Metrics& instance();

    void incEPCProcessed(int count = 1);
    void incEPCDropped(int count = 1);
    void recordDBLatency(double ms);
    void recordRequestLatency(double ms);
    void incRequestSuccess();
    void incRequestFailed();
    void setActiveReaders(int count);
    void setActiveTasks(int count);
    void recordTaskCompleted(const std::string& taskType, double durationMs);

    double getEPCPerSecond();
    double getDBLatencyP99();
    double getRequestSuccessRate();
    double getErrorRate();
    int getActiveReaders();
    int getActiveTasks();

    std::unordered_map<std::string, std::string> getAllMetrics();
    void reset();

    using AlertCallback = std::function<void(const std::string& alertType, const std::string& message)>;
    void setAlertCallback(AlertCallback callback);

private:
    Metrics();
    ~Metrics() = default;
    Metrics(const Metrics&) = delete;
    Metrics& operator=(const Metrics&) = delete;

    void checkThresholds();

    std::atomic<int64_t> epcProcessed_{0};
    std::atomic<int64_t> epcDropped_{0};
    std::atomic<int64_t> requestsSuccess_{0};
    std::atomic<int64_t> requestsFailed_{0};
    std::atomic<int> activeReaders_{0};
    std::atomic<int> activeTasks_{0};

    std::vector<double> dbLatencies_;
    std::vector<double> requestLatencies_;
    std::mutex latencyMutex_;

    std::chrono::steady_clock::time_point startTime_;
    std::chrono::steady_clock::time_point lastEPCCountTime_;
    int64_t lastEPCProcessed_{0};

    AlertCallback alertCallback_;

    static constexpr double DB_LATENCY_THRESHOLD_MS = 100.0;
    static constexpr double ERROR_RATE_THRESHOLD = 0.05;
    static constexpr int ACTIVE_READERS_THRESHOLD = 0;
};