#include "task_scheduler.h"
#include "../core/logger.h"
#include <sstream>
#include <fstream>

namespace bi {

TaskScheduler::TaskScheduler() {}

TaskScheduler::~TaskScheduler() {
    stop();
}

TaskScheduler& TaskScheduler::instance() {
    static TaskScheduler instance;
    return instance;
}

void TaskScheduler::start() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (running_) {
        spdlog::warn("Task scheduler is already running");
        return;
    }
    
    running_ = true;
    schedulerThread_ = std::thread([this]() {
        schedulerLoop();
    });
    
    spdlog::info("Task scheduler started");
}

void TaskScheduler::stop() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!running_) {
        return;
    }
    
    running_ = false;
    if (schedulerThread_.joinable()) {
        schedulerThread_.join();
    }
    
    spdlog::info("Task scheduler stopped");
}

bool TaskScheduler::isRunning() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return running_;
}

std::string TaskScheduler::addTask(const InternalScheduledTask& task) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    InternalScheduledTask newTask = task;
    if (newTask.task_id.empty()) {
        newTask.task_id = "TASK-" + std::to_string(std::time(nullptr));
    }
    
    tasks_[newTask.task_id] = newTask;
    spdlog::info("Added scheduled task: {}", newTask.task_id);
    
    return newTask.task_id;
}

bool TaskScheduler::removeTask(const std::string& taskId) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = tasks_.find(taskId);
    if (it != tasks_.end()) {
        tasks_.erase(it);
        spdlog::info("Removed scheduled task: {}", taskId);
        return true;
    }
    return false;
}

bool TaskScheduler::updateTask(const std::string& taskId, const InternalScheduledTask& task) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = tasks_.find(taskId);
    if (it != tasks_.end()) {
        InternalScheduledTask updatedTask = task;
        updatedTask.task_id = taskId;
        tasks_[taskId] = updatedTask;
        spdlog::info("Updated scheduled task: {}", taskId);
        return true;
    }
    return false;
}

InternalScheduledTask TaskScheduler::getTask(const std::string& taskId) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = tasks_.find(taskId);
    if (it != tasks_.end()) {
        return it->second;
    }
    return InternalScheduledTask();
}

std::vector<InternalScheduledTask> TaskScheduler::getAllTasks() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<InternalScheduledTask> result;
    for (const auto& pair : tasks_) {
        result.push_back(pair.second);
    }
    return result;
}

std::vector<InternalScheduledTask> TaskScheduler::getEnabledTasks() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<InternalScheduledTask> result;
    for (const auto& pair : tasks_) {
        if (pair.second.enabled) {
            result.push_back(pair.second);
        }
    }
    return result;
}

void TaskScheduler::enableTask(const std::string& taskId) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = tasks_.find(taskId);
    if (it != tasks_.end()) {
        it->second.enabled = true;
        spdlog::info("Enabled scheduled task: {}", taskId);
    }
}

void TaskScheduler::disableTask(const std::string& taskId) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = tasks_.find(taskId);
    if (it != tasks_.end()) {
        it->second.enabled = false;
        spdlog::info("Disabled scheduled task: {}", taskId);
    }
}

void TaskScheduler::schedulerLoop() {
    while (running_) {
        auto now = std::chrono::system_clock::now();
        
        std::vector<InternalScheduledTask> tasksCopy;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            for (const auto& pair : tasks_) {
                if (pair.second.enabled) {
                    tasksCopy.push_back(pair.second);
                }
            }
        }
        
        for (const auto& task : tasksCopy) {
            std::chrono::system_clock::time_point nextRun;
            if (parseCron(task.cron_expression, nextRun)) {
                if (now >= nextRun) {
                    executeTask(task);
                }
            }
        }
        
        std::this_thread::sleep_for(std::chrono::minutes(1));
    }
}

bool TaskScheduler::parseCron(const std::string& cron, std::chrono::system_clock::time_point& nextRun) {
    if (cron == "0 9 * * *") {
        auto now = std::chrono::system_clock::now();
        auto nowTime = std::chrono::system_clock::to_time_t(now);
        std::tm localTime = *std::localtime(&nowTime);
        
        std::tm targetTime = localTime;
        targetTime.tm_hour = 9;
        targetTime.tm_min = 0;
        targetTime.tm_sec = 0;
        
        auto target = std::chrono::system_clock::from_time_t(std::mktime(&targetTime));
        
        if (target <= now) {
            target += std::chrono::hours(24);
        }
        
        nextRun = target;
        return true;
    }
    
    if (cron == "0 9 * * 1") {
        auto now = std::chrono::system_clock::now();
        auto nowTime = std::chrono::system_clock::to_time_t(now);
        std::tm localTime = *std::localtime(&nowTime);
        
        std::tm targetTime = localTime;
        targetTime.tm_hour = 9;
        targetTime.tm_min = 0;
        targetTime.tm_sec = 0;
        
        int daysUntilMonday = (8 - localTime.tm_wday) % 7;
        if (daysUntilMonday == 0) daysUntilMonday = 7;
        
        targetTime.tm_mday += daysUntilMonday;
        
        nextRun = std::chrono::system_clock::from_time_t(std::mktime(&targetTime));
        return true;
    }
    
    if (cron == "0 9 1 * *") {
        auto now = std::chrono::system_clock::now();
        auto nowTime = std::chrono::system_clock::to_time_t(now);
        std::tm localTime = *std::localtime(&nowTime);
        
        std::tm targetTime = localTime;
        targetTime.tm_hour = 9;
        targetTime.tm_min = 0;
        targetTime.tm_sec = 0;
        targetTime.tm_mday = 1;
        
        if (targetTime.tm_mon == localTime.tm_mon && localTime.tm_mday > 1) {
            targetTime.tm_mon++;
        }
        
        nextRun = std::chrono::system_clock::from_time_t(std::mktime(&targetTime));
        return true;
    }
    
    return false;
}

void TaskScheduler::executeTask(const InternalScheduledTask& task) {
    spdlog::info("Executing scheduled task: {}", task.task_id);
    
    try {
        InternalReportResult report = ReportService::instance().generateReport(task.report_type, task.filter);
        std::string exportData = ReportService::instance().exportReport(report, task.format);
        
        std::string filename = "report_" + report.report_id + ".csv";
        std::ofstream file(filename);
        file << exportData;
        file.close();
        
        spdlog::info("Task {} completed, report saved to {}", task.task_id, filename);
        
    } catch (const std::exception& e) {
        spdlog::error("Failed to execute task {}: {}", task.task_id, e.what());
    }
}

} // namespace bi