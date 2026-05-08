#include "edge_scheduler.h"

namespace edge_runtime {

EdgeScheduler& EdgeScheduler::instance() {
    static EdgeScheduler scheduler;
    return scheduler;
}

void EdgeScheduler::initialize() {
    pending_tasks_.clear();
    running_tasks_.clear();
}

void EdgeScheduler::shutdown() {
    pending_tasks_.clear();
    running_tasks_.clear();
}

void EdgeScheduler::tick() {
    while (running_tasks_.size() < max_concurrent_tasks_ && !pending_tasks_.empty()) {
        executeNextTask();
    }
    
    for (auto it = running_tasks_.begin(); it != running_tasks_.end(); ) {
        if (!it->running) {
            it = running_tasks_.erase(it);
        } else {
            ++it;
        }
    }
}

uint64_t EdgeScheduler::scheduleTask(EdgeTaskType type, uint64_t priority, uint64_t deadline_ms) {
    EdgeTask task;
    task.task_id = pending_tasks_.size() + running_tasks_.size() + 1;
    task.type = type;
    task.priority = priority;
    task.deadline_ms = deadline_ms;
    task.running = false;
    task.start_time = 0;
    
    pending_tasks_.push_back(task);
    
    std::sort(pending_tasks_.begin(), pending_tasks_.end(),
        [](const EdgeTask& a, const EdgeTask& b) {
            if (a.priority != b.priority) {
                return a.priority > b.priority;
            }
            return a.deadline_ms < b.deadline_ms;
        });
    
    return task.task_id;
}

bool EdgeScheduler::cancelTask(uint64_t task_id) {
    auto it = std::find_if(pending_tasks_.begin(), pending_tasks_.end(),
        [task_id](const EdgeTask& t) { return t.task_id == task_id; });
    if (it != pending_tasks_.end()) {
        pending_tasks_.erase(it);
        return true;
    }
    
    auto running_it = std::find_if(running_tasks_.begin(), running_tasks_.end(),
        [task_id](const EdgeTask& t) { return t.task_id == task_id; });
    if (running_it != running_tasks_.end()) {
        running_it->running = false;
        return true;
    }
    
    return false;
}

bool EdgeScheduler::isTaskRunning(uint64_t task_id) const {
    for (const auto& task : running_tasks_) {
        if (task.task_id == task_id) {
            return task.running;
        }
    }
    return false;
}

size_t EdgeScheduler::getPendingTaskCount() const {
    return pending_tasks_.size();
}

void EdgeScheduler::setMaxConcurrentTasks(size_t max) {
    max_concurrent_tasks_ = max;
}

void EdgeScheduler::executeNextTask() {
    if (pending_tasks_.empty()) {
        return;
    }
    
    EdgeTask task = pending_tasks_.front();
    pending_tasks_.erase(pending_tasks_.begin());
    
    task.running = true;
    task.start_time = 0;
    running_tasks_.push_back(task);
}

} // namespace edge_runtime