#pragma once

#include "truth_gate.h"
#include "../spatial_estimate/spatial_estimate.h"
#include <queue>
#include <mutex>
#include <thread>
#include <atomic>
#include <condition_variable>
#include <map>
#include <optional>

namespace pa::spatial_truth {

class TruthGateAsyncRunner {
public:
    static TruthGateAsyncRunner& instance();

    void start();
    void stop();
    void submit(const SpatialEstimate& estimate, uint64_t frame_id);
    std::optional<TruthGateResult> getResult(uint64_t frame_id);

    void setQueueSize(size_t size) { max_queue_size_ = size; }

private:
    TruthGateAsyncRunner() = default;

    struct Task {
        SpatialEstimate estimate;
        uint64_t frame_id;
    };

    std::queue<Task> queue_;
    mutable std::mutex queue_mutex_;
    std::mutex result_mutex_;
    std::map<uint64_t, TruthGateResult> results_;
    std::atomic<bool> running_{false};
    std::thread worker_;
    std::condition_variable cv_;
    size_t max_queue_size_ = 1024;

    void workerLoop();
};

} // namespace pa::spatial_truth