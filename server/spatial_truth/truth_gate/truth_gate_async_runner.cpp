#include "truth_gate_async_runner.h"

namespace pa::spatial_truth {

TruthGateAsyncRunner& TruthGateAsyncRunner::instance() {
    static TruthGateAsyncRunner inst;
    return inst;
}

void TruthGateAsyncRunner::start() {
    if (running_.load()) return;
    running_.store(true);
    worker_ = std::thread(&TruthGateAsyncRunner::workerLoop, this);
}

void TruthGateAsyncRunner::stop() {
    running_.store(false);
    cv_.notify_all();
    if (worker_.joinable()) worker_.join();
}

void TruthGateAsyncRunner::submit(const SpatialEstimate& estimate, uint64_t frame_id) {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    if (queue_.size() >= max_queue_size_) {
        queue_.pop();
    }
    queue_.push({estimate, frame_id});
    cv_.notify_one();
}

std::optional<TruthGateResult> TruthGateAsyncRunner::getResult(uint64_t frame_id) {
    std::lock_guard<std::mutex> lock(result_mutex_);
    auto it = results_.find(frame_id);
    if (it == results_.end()) return std::nullopt;
    auto result = it->second;
    results_.erase(it);
    return result;
}

void TruthGateAsyncRunner::workerLoop() {
    TruthGate gate;
    while (running_.load()) {
        Task task;
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            cv_.wait(lock, [this] { return !queue_.empty() || !running_.load(); });
            if (!running_.load()) break;
            if (queue_.empty()) continue;
            task = queue_.front();
            queue_.pop();
        }

        auto result = gate.evaluate(task.estimate, task.frame_id);

        {
            std::lock_guard<std::mutex> lock(result_mutex_);
            results_[task.frame_id] = result;
        }
    }
}

} // namespace pa::spatial_truth