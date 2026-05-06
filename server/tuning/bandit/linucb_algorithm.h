#pragma once

#include "bandit_types.h"
#include <unordered_map>
#include <random>
#include <mutex>
#include <atomic>

namespace bandit {

class LinUCBAlgorithm {
public:
    LinUCBAlgorithm(int feature_dim = 14, double alpha = 1.5)
        : feature_dim_(feature_dim),
          alpha_(alpha),
          epsilon_(0.1),
          epsilon_min_(0.05),
          total_samples_(0),
          rng_(std::random_device{}()) {
        initializeArms();
    }
    
    // 设置探索率
    void setEpsilon(double eps) { epsilon_ = eps; }
    void setAlpha(double alpha) { alpha_ = alpha; }
    
    // 获取当前探索率（随时间衰减）
    double getCurrentEpsilon() const {
        double decay = std::max(0.0, 1.0 - total_samples_ / 10000.0);
        return std::max(epsilon_min_, epsilon_ * decay);
    }
    
    // 根据上下文选择动作
    Action selectAction(const ContextFeatures& context) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        Eigen::VectorXd x = context.toVector();
        
        // epsilon-greedy 探索
        if (std::uniform_real_distribution<double>(0, 1)(rng_) < getCurrentEpsilon()) {
            return selectRandomAction();
        }
        
        // LinUCB 选择
        double best_score = -1e9;
        Action best_action;
        
        for (auto& arm : arms_) {
            try {
                Eigen::VectorXd theta = arm.A.inverse() * arm.b;
                
                double exploit = theta.transpose() * x;
                double explore = alpha_ * std::sqrt(x.transpose() * arm.A.inverse() * x);
                double score = exploit + explore;
                
                if (score > best_score) {
                    best_score = score;
                    best_action = arm.action;
                }
            } catch (...) {
                // 矩阵奇异时跳过
                continue;
            }
        }
        
        return best_action;
    }
    
    // 更新模型（v3.3 - 含动态成本、漏检惩罚、Soft Constraint）
    void updateV33(const ContextFeatures& context, const Action& action, double reward, 
                    bool actually_missing = false,
                    double inspect_ratio = 0.1, double no_action_ratio = 0.6) {
        std::lock_guard<std::mutex> lock(mutex_);

        // 计算综合风险
        double risk = 0.5 * context.missing_risk + 
                       0.3 * context.inactivity_risk + 
                       0.2 * context.abnormal_risk;

        // 1. 比例惩罚（软惩罚）而非硬扣
        double action_cost = getActionCost(action.type, risk);
        double net_reward = reward * (1.0 - action_cost);

        // 2. 漏检惩罚（关键）
        if (action.type == ActionType::NO_ACTION && actually_missing) {
            net_reward -= 0.5;
        }

        // 3. Action 分布保护 - Soft Constraint
        double distribution_penalty = getDistributionPenalty(inspect_ratio, no_action_ratio, 0.2);
        net_reward += distribution_penalty;

        Eigen::VectorXd x = context.toVector();

        for (auto& arm : arms_) {
            if (arm.action.type == action.type && 
                std::abs(arm.action.threshold_adjustment - action.threshold_adjustment) < 0.01) {

                arm.A += x * x.transpose();
                arm.b += net_reward * x;
                arm.sample_count++;
                total_samples_++;
                break;
            }
        }
    }

    // 更新模型（v3.2 向后兼容接口）
    void update(const ContextFeatures& context, const Action& action, double reward) {
        std::lock_guard<std::mutex> lock(mutex_);

        // 加入行动成本惩罚
        double action_cost = getActionCost(action.type);
        double net_reward = reward - action_cost;

        Eigen::VectorXd x = context.toVector();

        for (auto& arm : arms_) {
            if (arm.action.type == action.type && 
                std::abs(arm.action.threshold_adjustment - action.threshold_adjustment) < 0.01) {

                arm.A += x * x.transpose();
                arm.b += net_reward * x;
                arm.sample_count++;
                total_samples_++;
                break;
            }
        }
    }
    
    // 获取动作的预测奖励
    double predictReward(const ContextFeatures& context, const Action& action) const {
        std::lock_guard<std::mutex> lock(mutex_);
        
        Eigen::VectorXd x = context.toVector();
        
        for (const auto& arm : arms_) {
            if (arm.action.type == action.type) {
                try {
                    Eigen::VectorXd theta = arm.A.inverse() * arm.b;
                    return theta.transpose() * x;
                } catch (...) {
                    return 0.0;
                }
            }
        }
        return 0.0;
    }
    
    // 获取所有动作的预测
    std::vector<std::pair<Action, double>> getAllPredictions(const ContextFeatures& context) const {
        std::vector<std::pair<Action, double>> results;
        Eigen::VectorXd x = context.toVector();
        
        for (const auto& arm : arms_) {
            try {
                Eigen::VectorXd theta = arm.A.inverse() * arm.b;
                double reward = theta.transpose() * x;
                results.emplace_back(arm.action, reward);
            } catch (...) {
                results.emplace_back(arm.action, 0.0);
            }
        }
        
        return results;
    }

    // v3.3: 获取 Top-K 排序动作（带 Uncertainty 和 Confidence）
    std::vector<RankedAction> getTopKRankedActions(const ContextFeatures& context, int k = 2) const {
        std::lock_guard<std::mutex> lock(mutex_);
        std::vector<RankedAction> results;
        Eigen::VectorXd x = context.toVector();

        double min_score = 1e9;
        double max_score = -1e9;

        // 第一遍：计算所有分数，找出 min 和 max 用于归一化
        for (const auto& arm : arms_) {
            try {
                Eigen::VectorXd theta = arm.A.inverse() * arm.b;
                double score = theta.transpose() * x;
                if (score < min_score) min_score = score;
                if (score > max_score) max_score = score;
            } catch (...) {
            }
        }

        // 避免除零
        double score_range = (max_score - min_score > 0) ? (max_score - min_score) : 1.0;

        // 第二遍：计算完整的 RankedAction
        for (const auto& arm : arms_) {
            try {
                Eigen::VectorXd theta = arm.A.inverse() * arm.b;
                double exploit = theta.transpose() * x;
                double uncertainty = std::sqrt(x.transpose() * arm.A.inverse() * x);
                double score = exploit + alpha_ * uncertainty;

                // 归一化分数到 [0, 1] 作为 confidence
                double confidence = (score - min_score) / score_range;
                confidence = std::max(0.0, std::min(1.0, confidence));

                RankedAction ra;
                ra.action = arm.action;
                ra.score = score;
                ra.uncertainty = uncertainty;
                ra.confidence = confidence;
                results.push_back(ra);
            } catch (...) {
                // 异常情况，填充默认值
                RankedAction ra;
                ra.action = arm.action;
                ra.score = 0.0;
                ra.uncertainty = 1.0;
                ra.confidence = 0.0;
                results.push_back(ra);
            }
        }

        // 按分数降序排序
        std::sort(results.begin(), results.end(),
                  [](const auto& a, const auto& b) { return a.score > b.score; });

        if (results.size() > static_cast<size_t>(k)) {
            results.resize(k);
        }

        return results;
    }
    
    // 获取模型参数用于持久化
    std::vector<ArmModel> getModels() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return arms_;
    }
    
    // 加载模型参数
    void loadModels(const std::vector<ArmModel>& models) {
        std::lock_guard<std::mutex> lock(mutex_);
        arms_ = models;
    }
    
private:
    void initializeArms() {
        // 为每个动作类型创建 arm
        std::vector<Action> base_actions = {
            {ActionType::NO_ACTION, 0.0, 1.0},
            {ActionType::INSPECT, 0.0, 1.0},
            {ActionType::ALERT, 0.0, 1.0},
            {ActionType::REALLOCATE, 0.0, 1.0}
        };
        
        for (const auto& action : base_actions) {
            arms_.emplace_back(feature_dim_, action);
        }
    }
    
    Action selectRandomAction() {
        std::uniform_int_distribution<int> dist(0, arms_.size() - 1);
        return arms_[dist(rng_)].action;
    }
    
    int feature_dim_;
    double alpha_;          // UCB 置信系数
    double epsilon_;        // 初始探索率
    double epsilon_min_;    // 最小探索率
    std::atomic<int> total_samples_;
    
    std::vector<ArmModel> arms_;
    mutable std::mutex mutex_;
    mutable std::mt19937 rng_;
};

} // namespace bandit