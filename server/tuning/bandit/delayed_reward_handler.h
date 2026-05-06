#pragma once

#include "bandit_types.h"
#include <chrono>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <unordered_map>
#include <atomic>

namespace bandit {

class DelayedRewardHandler {
public:
    static DelayedRewardHandler& instance();
    
    // 启动处理器线程
    void start();
    
    // 停止处理器线程
    void stop();
    
    // 添加延迟奖励
    void addDelayedReward(int decision_id, double reward, int delay_hours);
    
    // 取消延迟奖励
    void cancelReward(int decision_id);
    
    // 更新延迟奖励（用于修正）
    void updateReward(int decision_id, double new_reward);
    
    // 获取待处理的奖励数量
    size_t getPendingCount() const;
    
private:
    DelayedRewardHandler();
    ~DelayedRewardHandler();
    
    DelayedRewardHandler(const DelayedRewardHandler&) = delete;
    DelayedRewardHandler& operator=(const DelayedRewardHandler&) = delete;
    
    // 工作线程主循环
    void workerLoop();
    
    // 应用奖励回调
    void applyReward(int decision_id, double reward);
    
    struct PendingReward {
        double reward;
        std::chrono::system_clock::time_point trigger_time;
        std::string status;  // pending, processed, expired
    };
    
    std::unordered_map<int, PendingReward> pending_rewards_;
    mutable std::mutex mutex_;
    std::condition_variable cv_;
    std::thread worker_thread_;
    std::atomic<bool> running_;
};

} // namespace bandit