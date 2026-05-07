#pragma once
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <mutex>
#include <chrono>
#include <thread>
#include <functional>
#include "bi_service.h"

namespace bi {

class TaskScheduler {
public:
    static TaskScheduler& instance();

    void start();
    void stop();
    bool isRunning() const;

    std::string addTask(const InternalScheduledTask& task);
    bool removeTask(const std::string& taskId);
    bool updateTask(const std::string& taskId, const InternalScheduledTask& task);
    InternalScheduledTask getTask(const std::string& taskId) const;
    std::vector<InternalScheduledTask> getAllTasks() const;
    std::vector<InternalScheduledTask> getEnabledTasks() const;

    void enableTask(const std::string& taskId);
    void disableTask(const std::string& taskId);

private:
    TaskScheduler();
    ~TaskScheduler();
    TaskScheduler(const TaskScheduler&) = delete;
    TaskScheduler& operator=(const TaskScheduler&) = delete;

    void schedulerLoop();
    bool parseCron(const std::string& cron, std::chrono::system_clock::time_point& nextRun);
    void executeTask(const InternalScheduledTask& task);

    std::unordered_map<std::string, InternalScheduledTask> tasks_;
    mutable std::mutex mutex_;
    std::thread schedulerThread_;
    bool running_ = false;
};

} // namespace bi