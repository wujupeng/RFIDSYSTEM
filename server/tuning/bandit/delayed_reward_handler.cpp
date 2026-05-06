#include "delayed_reward_handler.h"
#include "bandit_engine.h"
#include <iostream>

namespace bandit {

DelayedRewardHandler& DelayedRewardHandler::instance() {
    static DelayedRewardHandler instance;
    return instance;
}

DelayedRewardHandler::DelayedRewardHandler()
    : running_(false) {
}

DelayedRewardHandler::~DelayedRewardHandler() {
    stop();
}

void DelayedRewardHandler::start() {
    if (!running_) {
        running_ = true;
        worker_thread_ = std::thread(&DelayedRewardHandler::workerLoop, this);
    }
}

void DelayedRewardHandler::stop() {
    if (running_) {
        running_ = false;
        cv_.notify_all();
        if (worker_thread_.joinable()) {
            worker_thread_.join();
        }
    }
}

void DelayedRewardHandler::addDelayedReward(int decision_id, double reward, int delay_hours) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto trigger_time = std::chrono::system_clock::now() + 
                        std::chrono::hours(delay_hours);
    
    pending_rewards_[decision_id] = {
        reward,
        trigger_time,
        "pending"
    };
    
    cv_.notify_all();
}

void DelayedRewardHandler::cancelReward(int decision_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    pending_rewards_.erase(decision_id);
}

void DelayedRewardHandler::updateReward(int decision_id, double new_reward) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = pending_rewards_.find(decision_id);
    if (it != pending_rewards_.end()) {
        it->second.reward = new_reward;
    }
}

size_t DelayedRewardHandler::getPendingCount() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return pending_rewards_.size();
}

void DelayedRewardHandler::workerLoop() {
    while (running_) {
        std::unique_lock<std::mutex> lock(mutex_);
        
        // 找到最早需要处理的奖励
        auto now = std::chrono::system_clock::now();
        auto earliest_time = std::chrono::system_clock::time_point::max();
        
        for (const auto& pair : pending_rewards_) {
            if (pair.second.status == "pending" && pair.second.trigger_time < earliest_time) {
                earliest_time = pair.second.trigger_time;
            }
        }
        
        if (earliest_time != std::chrono::system_clock::time_point::max()) {
            // 等待到触发时间或被唤醒
            cv_.wait_until(lock, earliest_time, [this]() { return !running_; });
        } else {
            // 没有待处理的奖励，等待新的奖励添加
            cv_.wait(lock, [this]() { return !running_ || !pending_rewards_.empty(); });
        }
        
        if (!running_) break;
        
        // 处理到期的奖励
        now = std::chrono::system_clock::now();
        std::vector<int> to_process;
        
        for (const auto& pair : pending_rewards_) {
            if (pair.second.status == "pending" && pair.second.trigger_time <= now) {
                to_process.push_back(pair.first);
            }
        }
        
        lock.unlock();
        
        // 在锁外应用奖励
        for (int decision_id : to_process) {
            lock.lock();
            auto it = pending_rewards_.find(decision_id);
            if (it != pending_rewards_ && it->second.status == "pending") {
                double reward = it->second.reward;
                it->second.status = "processed";
                lock.unlock();
                
                applyReward(decision_id, reward);
                
                lock.lock();
                pending_rewards_.erase(decision_id);
            }
            lock.unlock();
        }
    }
}

void DelayedRewardHandler::applyReward(int decision_id, double reward) {
    try {
        BanditEngine::instance().updateReward(decision_id, reward);
    } catch (const std::exception& e) {
        std::cerr << "[DelayedRewardHandler] Failed to apply reward: " << e.what() << std::endl;
    }
}

} // namespace bandit