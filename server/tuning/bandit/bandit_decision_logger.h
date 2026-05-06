#pragma once

#include "bandit_engine.h"
#include "../decision_repository.h"
#include <spdlog/spdlog.h>

namespace bandit {

// v3.3: Bandit 决策记录器 - 用于 SHADOW 模式
class BanditDecisionLogger {
public:
    static BanditDecisionLogger& instance() {
        static BanditDecisionLogger instance;
        return instance;
    }

    // 记录 SHADOW 模式决策
    void logShadowDecision(
        const ContextFeatures& context,
        const PolicyMixer::MixedDecision& decision,
        const Action& ruleAction,
        int asset_id = -1) {

        if (decision.source == "GUARDRAIL") {
            spdlog::info("[SHADOW-GUARDRAIL] asset_id={}, action={}, reason={}, "
                         "bandit_action={}, rule_action={}",
                         asset_id,
                         actionTypeToString(decision.final_action.type),
                         decision.reason,
                         actionTypeToString(decision.final_action.type),
                         actionTypeToString(ruleAction.type));
        } else {
            bool matches = (decision.final_action.type == ruleAction.type);
            spdlog::info("[SHADOW-{}] asset_id={}, action={}, confidence={:.4f}, "
                         "uncertainty={:.4f}, rule_action={}, match={}",
                         decision.source,
                         asset_id,
                         actionTypeToString(decision.final_action.type),
                         decision.top_k_actions.empty() ? 0.0 : decision.top_k_actions[0].confidence,
                         decision.top_k_actions.empty() ? 0.0 : decision.top_k_actions[0].uncertainty,
                         actionTypeToString(ruleAction.type),
                         matches ? "YES" : "NO");
        }

        // 如果有数据库支持，可以记录到决策表
        recordToDatabase(context, decision, ruleAction, asset_id);
    }

    // 获取 SHADOW 模式统计
    struct ShadowStats {
        int total_decisions;
        int guardrail_high_risk;
        int guardrail_illegal_location;
        int guardrail_low_confidence;
        int from_bandit;
        int from_rule;
        int from_top_k;
        int matches_rule;
        int differs_rule;
        double avg_confidence;
        double avg_uncertainty;
    };

    ShadowStats getStats() const {
        ShadowStats stats = {0};
        auto engineStats = BanditEngine::instance().getStatistics();

        stats.total_decisions = engineStats.total_decisions;
        stats.guardrail_high_risk = engineStats.guardrail_triggered_high_risk;
        stats.guardrail_illegal_location = engineStats.guardrail_triggered_illegal_location;
        stats.guardrail_low_confidence = engineStats.guardrail_triggered_low_confidence;
        stats.avg_confidence = engineStats.avg_confidence;
        stats.avg_uncertainty = engineStats.avg_uncertainty;

        return stats;
    }

    // 打印统计报告
    void printReport() {
        auto stats = getStats();

        spdlog::info("═══════════════════════════════════════════════════════════════");
        spdlog::info("                    SHADOW 模式统计报告                       ");
        spdlog::info("═══════════════════════════════════════════════════════════════");
        spdlog::info("总决策数: {}", stats.total_decisions);
        spdlog::info("");
        spdlog::info("【Guardrail 触发】");
        spdlog::info("  高风险触发: {}", stats.guardrail_high_risk);
        spdlog::info("  非法位置触发: {}", stats.guardrail_illegal_location);
        spdlog::info("  低置信度触发: {}", stats.guardrail_low_confidence);
        spdlog::info("");
        spdlog::info("【决策来源】");
        spdlog::info("  Bandit: {}", stats.from_bandit);
        spdlog::info("  Rule: {}", stats.from_rule);
        spdlog::info("  Top-K: {}", stats.from_top_k);
        spdlog::info("");
        spdlog::info("【与规则一致性】");
        spdlog::info("  一致: {}", stats.matches_rule);
        spdlog::info("  不同: {}", stats.differs_rule);
        if (stats.total_decisions > 0) {
            double matchRate = (double)stats.matches_rule / stats.total_decisions * 100;
            spdlog::info("  一致率: {:.1f}%", matchRate);
        }
        spdlog::info("");
        spdlog::info("【模型置信度】");
        spdlog::info("  平均置信度: {:.4f}", stats.avg_confidence);
        spdlog::info("  平均不确定性: {:.4f}", stats.avg_uncertainty);
        spdlog::info("═══════════════════════════════════════════════════════════════");
    }

private:
    BanditDecisionLogger() = default;

    void recordToDatabase(const ContextFeatures& context,
                          const PolicyMixer::MixedDecision& decision,
                          const Action& ruleAction,
                          int asset_id) {
        // TODO: 如果有 decision_repository 支持，可以在这里记录
        // decision_repository.saveShadowDecision(context, decision, ruleAction);
    }

    std::string actionTypeToString(ActionType type) {
        switch (type) {
            case ActionType::NO_ACTION: return "NO_ACTION";
            case ActionType::INSPECT: return "INSPECT";
            case ActionType::ALERT: return "ALERT";
            case ActionType::REALLOCATE: return "REALLOCATE";
            default: return "UNKNOWN";
        }
    }
};

} // namespace bandit
