#pragma once

#include "../spatial_positioning/spatial_types.h"
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>

struct LocalizationTask {
    std::string epc;
    std::vector<TagObservation> observations;
    std::function<void(TagPosition)> callback;
};

class LocalizationWorkerPool {
public:
    LocalizationWorkerPool(int num_workers = 4);
    
    ~LocalizationWorkerPool();
    
    void submitTask(const LocalizationTask& task);
    
    void start();
    
    void stop();
    
    size_t getPendingTasks() const;
    
private:
    void workerLoop();
    
    std::vector<std::thread> workers_;
    std::queue<LocalizationTask> tasks_;
    std::mutex mutex_;
    std::condition_variable cv_;
    bool running_ = false;
};