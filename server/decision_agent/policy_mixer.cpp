#include "policy_mixer.h"
#include "../core/logger.h"
#include <algorithm>

namespace v33 {

PolicyMixer::PolicyMixer() {
    spdlog::info("PolicyMixer initialized");
}

DecisionResult PolicyMixer::decide(
    const RiskScore& risk,
    bool in_illegal_location,
    const std::vector<RankedAction>& bandit_actions,
    bool shadow_mode
) {
    DecisionResult result;
    result.is_shadow_mode = shadow_mode;
    result.candidates = bandit_actions;

    auto trigger = Guardrail::checkTrigger(risk, in_illegal_location);

    if (trigger != GuardrailTrigger::NONE) {
        spdlog::debug("PolicyMixer: Guardrail triggered - {}", Guardrail::getReason(trigger));
        return makeGuardrailDecision(trigger, shadow_mode);
    }

    if (bandit_actions.empty()) {
        spdlog::debug("PolicyMixer: No bandit actions, defaulting to NO_ACTION");
        result.final_action = ActionType::NO_ACTION;
        result.reason = "NO_BANDIT_ACTIONS";
        result.confidence = 0.0;
        result.trigger = GuardrailTrigger::NONE;
        return result;
    }

    if (bandit_actions[0].uncertainty > HIGH_UNCERTAINTY_THRESHOLD) {
        spdlog::debug("PolicyMixer: High uncertainty ({})", bandit_actions[0].uncertainty);
        return makeUncertaintyFallback(bandit_actions, shadow_mode);
    }

    if (bandit_actions[0].confidence < LOW_CONFIDENCE_THRESHOLD) {
        spdlog::debug("PolicyMixer: Low confidence ({})", bandit_actions[0].confidence);
        return makeLowConfidenceDecision(bandit_actions, shadow_mode);
    }

    return makeBanditDecision(bandit_actions, shadow_mode);
}

DecisionResult PolicyMixer::makeGuardrailDecision(GuardrailTrigger trigger, bool shadow_mode) {
    DecisionResult result;
    result.is_shadow_mode = shadow_mode;
    result.trigger = trigger;
    result.reason = Guardrail::getReason(trigger);

    switch (trigger) {
        case GuardrailTrigger::ILLEGAL_LOCATION:
            result.final_action = ActionType::ALERT;
            result.confidence = 1.0;
            break;
        case GuardrailTrigger::HIGH_RISK_MISSING:
        case GuardrailTrigger::HIGH_RISK_INACTIVITY:
            result.final_action = ActionType::INSPECT;
            result.confidence = 1.0;
            break;
        default:
            result.final_action = ActionType::NO_ACTION;
            result.confidence = 0.0;
    }

    return result;
}

DecisionResult PolicyMixer::makeBanditDecision(const std::vector<RankedAction>& actions, bool shadow_mode) {
    DecisionResult result;
    result.is_shadow_mode = shadow_mode;
    result.trigger = GuardrailTrigger::NONE;
    result.final_action = actions[0].action;
    result.candidates = actions;
    result.confidence = actions[0].confidence;
    result.reason = "BANDIT_DECISION";

    spdlog::debug("PolicyMixer: Bandit decision - {} (conf={})",
        static_cast<int>(actions[0].action), actions[0].confidence);

    return result;
}

DecisionResult PolicyMixer::makeUncertaintyFallback(const std::vector<RankedAction>& actions, bool shadow_mode) {
    DecisionResult result;
    result.is_shadow_mode = shadow_mode;
    result.trigger = GuardrailTrigger::HIGH_UNCERTAINTY;
    result.final_action = ActionType::INSPECT;
    result.candidates = actions;
    result.confidence = 0.5;
    result.reason = Guardrail::getReason(GuardrailTrigger::HIGH_UNCERTAINTY);

    spdlog::info("PolicyMixer: Uncertainty fallback to INSPECT");

    return result;
}

DecisionResult PolicyMixer::makeLowConfidenceDecision(const std::vector<RankedAction>& actions, bool shadow_mode) {
    DecisionResult result;
    result.is_shadow_mode = shadow_mode;
    result.trigger = GuardrailTrigger::LOW_CONFIDENCE;
    result.candidates = actions;
    result.reason = Guardrail::getReason(GuardrailTrigger::LOW_CONFIDENCE);

    if (!actions.empty()) {
        result.final_action = actions[0].action;
        result.confidence = actions[0].confidence;
    } else {
        result.final_action = ActionType::NO_ACTION;
        result.confidence = 0.0;
    }

    spdlog::info("PolicyMixer: Low confidence decision - showing Top-K");

    return result;
}

}
