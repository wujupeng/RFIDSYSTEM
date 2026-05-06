#pragma once

#include "bandit_types.h"

namespace bandit {

// v3.3: Guardrail 模块 - 生产级安全兜底
class GuardrailModule {
public:
    // 检查是否触发 Guardrail
    static GuardrailDecision checkGuardrail(const ContextFeatures& context, double bandit_confidence = 1.0) {
        // 1. 高风险检查：missing_risk > 0.9 → 强制 INSPECT
        if (context.missing_risk > 0.9) {
            return {
                {ActionType::INSPECT, 0.0, 1.0},
                GuardrailType::HIGH_RISK,
                "High missing risk (" + std::to_string(context.missing_risk) + " > 0.9)",
                true
            };
        }

        // 2. 非法位置检查：in_illegal_location → 强制 ALERT
        if (context.in_illegal_location) {
            return {
                {ActionType::ALERT, 0.0, 1.0},
                GuardrailType::ILLEGAL_LOCATION,
                "Asset in illegal location",
                true
            };
        }

        // 3. 低置信度检查：confidence < 0.6 → 标记（实际处理由 PolicyMixer 决定）
        if (bandit_confidence < 0.6) {
            return {
                {ActionType::NO_ACTION, 0.0, 1.0},
                GuardrailType::LOW_CONFIDENCE,
                "Bandit confidence too low (" + std::to_string(bandit_confidence) + " < 0.6)",
                true
            };
        }

        // 无 Guardrail 触发
        return {
            {ActionType::NO_ACTION, 0.0, 1.0},
            GuardrailType::NONE,
            "No guardrail triggered",
            false
        };
    }
};

} // namespace bandit
