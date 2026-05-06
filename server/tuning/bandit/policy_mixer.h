#pragma once

#include "bandit_types.h"
#include "linucb_algorithm.h"
#include "guardrail_module.h"

namespace bandit {

// v3.3: PolicyMixer - 决策融合引擎（Hybrid Policy）
class PolicyMixer {
public:
    struct MixedDecision {
        Action final_action;
        std::string source;  // "GUARDRAIL", "RULE", "BANDIT", "TOP_K"
        std::string reason;
        std::vector<RankedAction> top_k_actions;  // Top-K 选项（如有）
    };

    // 主决策方法
    static MixedDecision decide(const ContextFeatures& context, LinUCBAlgorithm& bandit, int top_k = 2) {
        MixedDecision result;

        // 先获取 Bandit Top-K 动作（用于检查置信度）
        auto top_k_actions = bandit.getTopKRankedActions(context, top_k);
        result.top_k_actions = top_k_actions;

        double top1_confidence = top_k_actions.empty() ? 0.0 : top_k_actions[0].confidence;

        // Step 1: 检查 Guardrail（最高优先级）
        auto guardrail = GuardrailModule::checkGuardrail(context, top1_confidence);
        if (guardrail.triggered) {
            result.final_action = guardrail.action;
            result.source = "GUARDRAIL";
            result.reason = guardrail.getTypeString() + ": " + guardrail.reason;
            return result;
        }

        // Step 2: 检查不确定性
        double top1_uncertainty = top_k_actions.empty() ? 0.0 : top_k_actions[0].uncertainty;
        if (top1_uncertainty > 0.4) {
            // 高不确定性 → 显示 Top-K 给用户，默认选 Top1
            result.final_action = top_k_actions[0].action;
            result.source = "TOP_K";
            result.reason = "High uncertainty (" + std::to_string(top1_uncertainty) + " > 0.4), showing Top-K options";
            return result;
        }

        // Step 3: 检查置信度
        if (top1_confidence < 0.6) {
            // 低置信度 → 回退规则（这里模拟 RuleEngine 返回）
            result.final_action = simulateRuleEngine(context);
            result.source = "RULE";
            result.reason = "Low confidence (" + std::to_string(top1_confidence) + " < 0.6), fallback to rule engine";
            return result;
        }

        // Step 4: 正常情况 → 使用 Bandit
        result.final_action = top_k_actions[0].action;
        result.source = "BANDIT";
        result.reason = "Bandit decision with confidence " + std::to_string(top1_confidence);
        return result;
    }

private:
    // 模拟规则引擎（实际系统有完整 RuleEngine）
    static Action simulateRuleEngine(const ContextFeatures& context) {
        if (context.missing_risk > 0.7) {
            return {ActionType::INSPECT, 0.0, 1.0};
        } else if (context.missing_risk > 0.4) {
            return {ActionType::ALERT, 0.0, 1.0};
        } else {
            return {ActionType::NO_ACTION, 0.0, 1.0};
        }
    }
};

} // namespace bandit
