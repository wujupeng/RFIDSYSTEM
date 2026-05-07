#include "localization_worker_pool.h"
#include "../spatial_positioning/tag_position_engine.h"

LocalizationWorkerPool::LocalizationWorkerPool(int num_workers) {
    for (int i = 0; i < num_workers; ++i) {
        workers_.emplace_back(&LocalizationWorkerPool::workerLoop, this);
    }
}

LocalizationWorkerPool::~LocalizationWorkerPool() {
    stop();
}

void LocalizationWorkerPool::start() {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        running_ = true;
    }
    cv_.notify_all();
}

void LocalizationWorkerPool::stop() {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        running_ = false;
    }
    cv_.notify_all();
    
    for (auto& worker : workers_) {
        if (worker.joinable()) {
            worker.join();
        }
    }
}

void LocalizationWorkerPool::submitTask(const LocalizationTask& task) {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        tasks_.push(task);
    }
    cv_.notify_one();
}

void LocalizationWorkerPool::workerLoop() {
    while (true) {
        LocalizationTask task;
        
        {
            std::unique_lock<std::mutex> lock(mutex_);
            cv_.wait(lock, [this] { return !running_ || !tasks_.empty(); });
            
            if (!running_ && tasks_.empty()) {
                return;
            }
            
            task = tasks_.front();
            tasks_.pop();
        }
        
        auto& engine = TagPositionEngine::instance();
        
        for (const auto& obs : task.observations) {
            engine.addObservation(obs);
        }
        
        engine.update();
        
        TagPosition pos = engine.getPosition(task.epc);
        
        if (task.callback) {
            task.callback(pos);
        }
    }
}

size_t LocalizationWorkerPool::getPendingTasks() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return tasks_.size();
}