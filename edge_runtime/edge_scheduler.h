#pragma once

#include <cstdint>
#include <vector>
#include <map>

namespace edge_runtime {

enum class EdgeTaskType {
    LOCALIZATION,
    KALMAN_UPDATE,
    SPATIAL_FILTER,
    LOCAL_AI,
    DATA_CACHING,
    SYNC_PENDING
};

struct EdgeTask {
    uint64_t task_id;
    EdgeTaskType type;
    uint64_t priority;
    uint64_t deadline_ms;
    bool running;
    uint64_t start_time;
};

class EdgeScheduler {
public:
    static EdgeScheduler& instance();
    
    void initialize();
    
    void shutdown();
    
    void tick();
    
    uint64_t scheduleTask(EdgeTaskType type, uint64_t priority, uint64_t deadline_ms);
    
    bool cancelTask(uint64_t task_id);
    
    bool isTaskRunning(uint64_t task_id) const;
    
    size_t getPendingTaskCount() const;
    
    void setMaxConcurrentTasks(size_t max);
    
private:
    EdgeScheduler() = default;
    
    void executeNextTask();
    
    std::vector<EdgeTask> pending_tasks_;
    std::vector<EdgeTask> running_tasks_;
    size_t max_concurrent_tasks_ = 4;
};

} // namespace edge_runtime