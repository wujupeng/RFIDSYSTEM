#pragma once
#include <vector>
#include <string>
#include "../models/ranked_action.h"
#include "guardrail.h"

namespace v33 {

struct DecisionResult {
    ActionType final_action;
    std::vector<RankedAction> candidates;
    std::string reason;
    double confidence;
    GuardrailTrigger trigger;
    bool is_shadow_mode;
};

class PolicyMixer {
public:
    PolicyMixer();

    DecisionResult decide(
        const RiskScore& risk,
        bool in_illegal_location,
        const std::vector<RankedAction>& bandit_actions,
        bool shadow_mode = false
    );

    static constexpr double HIGH_UNCERTAINTY_THRESHOLD = 0.4;
    static constexpr double LOW_CONFIDENCE_THRESHOLD = 0.6;

private:
    DecisionResult makeGuardrailDecision(GuardrailTrigger trigger, bool shadow_mode);
    DecisionResult makeBanditDecision(const std::vector<RankedAction>& actions, bool shadow_mode);
    DecisionResult makeUncertaintyFallback(const std::vector<RankedAction>& actions, bool shadow_mode);
    DecisionResult makeLowConfidenceDecision(const std::vector<RankedAction>& actions, bool shadow_mode);
};

}
