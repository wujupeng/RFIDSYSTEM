#include "decision_agent.h"
#include "../core/logger.h"

namespace v33 {

DecisionAgent::DecisionAgent() : shadow_mode_enabled_(true) {
    spdlog::info("DecisionAgent initialized (Shadow Mode: {})",
                 shadow_mode_enabled_ ? "ENABLED" : "DISABLED");
}

DecisionAgent& DecisionAgent::instance() {
    static DecisionAgent instance;
    return instance;
}

DecisionResult DecisionAgent::makeDecision(
    int asset_id,
    const RiskScore& risk,
    bool illegal_location,
    const std::vector<RankedAction>& bandit_actions,
    bool shadow_mode
) {
    bool effective_shadow_mode = shadow_mode || shadow_mode_enabled_;

    spdlog::info("DecisionAgent: Processing asset {} (shadow={})",
                 asset_id, effective_shadow_mode ? "YES" : "NO");

    auto result = mixer_.decide(risk, illegal_location, bandit_actions, effective_shadow_mode);

    spdlog::info("DecisionAgent: Decision for asset {} - {} (reason: {}, conf: {:.2f})",
                 asset_id,
                 static_cast<int>(result.final_action),
                 result.reason,
                 result.confidence);

    if (result.is_shadow_mode) {
        spdlog::info("DecisionAgent: SHADOW MODE - Decision recorded but not executed");
    }

    return result;
}

}
