#pragma once
#include "policy_mixer.h"
#include "../models/ranked_action.h"
#include <vector>

namespace v33 {

class DecisionAgent {
public:
    static DecisionAgent& instance();

    DecisionResult makeDecision(
        int asset_id,
        const RiskScore& risk,
        bool illegal_location,
        const std::vector<RankedAction>& bandit_actions,
        bool shadow_mode = false
    );

    void setShadowMode(bool enable) { shadow_mode_enabled_ = enable; }
    bool isShadowModeEnabled() const { return shadow_mode_enabled_; }

private:
    DecisionAgent();
    PolicyMixer mixer_;
    bool shadow_mode_enabled_;
};

}
