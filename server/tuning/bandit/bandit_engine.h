#pragma once

#include "bandit_types.h"
#include "linucb_algorithm.h"
#include "guardrail_module.h"
#include "policy_mixer.h"
#include <memory>
#include <mutex>
#include <unordered_map>
#include <deque>
#include <atomic>
#include <string>

namespace bandit {

class BanditEngine {
public:
    static BanditEngine& instance();
    
    // 设置运行模式
    void setMode(BanditMode mode);
    BanditMode getMode() const { return mode_; }
    
    // 选择动作（v3.2 接口，向后兼容）
    Action selectAction(const ContextFeatures& context);
    
    // v3.3: 获取 Top-K 排序动作（含不确定性和置信度）
    std::vector<RankedAction> getTopKRankedActions(const ContextFeatures& context, int k = 2) const;
    
    // v3.3: 使用 PolicyMixer 进行混合决策
    PolicyMixer::MixedDecision decide(const ContextFeatures& context, int top_k = 2);
    
    // v3.3: 检查 Guardrail 状态
    GuardrailDecision checkGuardrail(const ContextFeatures& context, double confidence = 1.0) const;
    
    // 获取所有动作的预测（用于 SUGGESTION 模式）
    std::vector<std::pair<Action, double>> getAllPredictions(const ContextFeatures& context) const;
    
    // 记录决策（用于后续奖励更新）
    void recordDecision(int decision_id, const ContextFeatures& context, const Action& action, 
                        const std::string& decision_source = "BANDIT",
                        double confidence = 1.0, double uncertainty = 0.0);
    
    // 更新奖励（即时奖励）
    void updateReward(int decision_id, double reward);
    
    // v3.3: 更新奖励（含漏检惩罚和分布约束）
    void updateRewardV33(int decision_id, double reward, bool actually_missing = false,
                         double inspect_ratio = 0.1, double no_action_ratio = 0.6);
    
    // 添加延迟奖励
    void addDelayedReward(int decision_id, double reward, int delay_hours);
    
    // 处理延迟奖励（定时调用）
    void processDelayedRewards();
    
    // 获取统计信息
    struct Statistics {
        int total_decisions;
        int shadow_decisions;
        int suggestion_decisions;
        int auto_decisions;
        int guardrail_triggered_high_risk;
        int guardrail_triggered_illegal_location;
        int guardrail_triggered_low_confidence;
        double avg_reward;
        double avg_confidence;
        double avg_uncertainty;
        int model_updates;
    };
    Statistics getStatistics() const;
    
    // 获取 SHADOW 模式统计
    struct ShadowStats {
        int shadow_matches_rule;      // SHADOW 决策与规则一致
        int shadow_differs_from_rule; // SHADOW 决策与规则不同
        int shadow_guardrail_triggered;
    };
    ShadowStats getShadowStats() const;
    
    // 保存/加载模型
    void saveModels(const std::string& path);
    void loadModels(const std::string& path);
    
    // 重置模型
    void reset();
    
private:
    BanditEngine();
    ~BanditEngine();
    
    // 禁用拷贝
    BanditEngine(const BanditEngine&) = delete;
    BanditEngine& operator=(const BanditEngine&) = delete;
    
    // 获取特征维度
    int getFeatureDimension() const { return 14; }
    
    std::unique_ptr<LinUCBAlgorithm> algorithm_;
    BanditMode mode_;
    
    // 决策记录缓存（用于奖励更新）
    struct DecisionRecord {
        ContextFeatures context;
        Action action;
        std::string decision_source;
        double confidence;
        double uncertainty;
        bool reward_received;
    };
    std::unordered_map<int, DecisionRecord> decision_cache_;
    
    // 延迟奖励队列
    std::deque<DelayedReward> delayed_rewards_;
    
    // 统计信息
    std::atomic<int> total_decisions_;
    std::atomic<int> shadow_decisions_;
    std::atomic<int> suggestion_decisions_;
    std::atomic<int> auto_decisions_;
    std::atomic<int> guardrail_triggered_high_risk_;
    std::atomic<int> guardrail_triggered_illegal_location_;
    std::atomic<int> guardrail_triggered_low_confidence_;
    std::atomic<double> total_reward_;
    std::atomic<double> total_confidence_;
    std::atomic<double> total_uncertainty_;
    std::atomic<int> model_updates_;
    std::atomic<int> shadow_matches_rule_;
    std::atomic<int> shadow_differs_from_rule_;
    std::atomic<int> shadow_guardrail_triggered_;
    
    mutable std::mutex mutex_;
};

} // namespace bandit