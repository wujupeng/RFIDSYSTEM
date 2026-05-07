#include "reward_engine.h"
#include "../core/logger.h"

namespace v33 {

RewardEngine& RewardEngine::instance() {
    static RewardEngine instance;
    return instance;
}

double RewardEngine::computeReward(
    int decision_id,
    const DecisionOutcome& outcome,
    const std::string& action
) {
    auto result = computeDetailedReward(decision_id, outcome, action);
    return result.reward;
}

RewardResult RewardEngine::computeDetailedReward(
    int decision_id,
    const DecisionOutcome& outcome,
    const std::string& action
) {
    RewardResult result;
    result.reward = 0.0;
    result.true_positive = 0.0;
    result.false_positive = 0.0;
    result.false_negative = 0.0;
    result.true_negative = 0.0;
    result.reward_type = "NONE";

    bool action_was_inspect = (action == "INSPECT");
    bool action_was_no_action = (action == "NO_ACTION");

    if (action_was_no_action && outcome.actual_missing) {
        result.reward = PENALTY_FALSE_NEGATIVE;
        result.false_negative = 1.0;
        result.reward_type = "FALSE_NEGATIVE";
        spdlog::warn("RewardEngine: FALSE_NEGATIVE for decision {} - missed issue", decision_id);
    }
    else if (action_was_no_action && !outcome.actual_missing && !outcome.actual_issue) {
        result.reward = REWARD_TRUE_NEGATIVE;
        result.true_negative = 1.0;
        result.reward_type = "TRUE_NEGATIVE";
        spdlog::debug("RewardEngine: TRUE_NEGATIVE for decision {}", decision_id);
    }
    else if (action_was_inspect && outcome.actual_issue) {
        result.reward = REWARD_TRUE_POSITIVE;
        result.true_positive = 1.0;
        result.reward_type = "TRUE_POSITIVE";
        spdlog::info("RewardEngine: TRUE_POSITIVE for decision {} - caught issue", decision_id);
    }
    else if (action_was_inspect && !outcome.actual_issue) {
        result.reward = PENALTY_FALSE_POSITIVE;
        result.false_positive = 1.0;
        result.reward_type = "FALSE_POSITIVE";
        spdlog::warn("RewardEngine: FALSE_POSITIVE for decision {} - unnecessary inspect", decision_id);
    }
    else if (action == "ALERT" && outcome.actual_issue) {
        result.reward = REWARD_TRUE_POSITIVE * 1.2;
        result.true_positive = 1.0;
        result.reward_type = "TRUE_POSITIVE_ALERT";
        spdlog::info("RewardEngine: TRUE_POSITIVE_ALERT for decision {}", decision_id);
    }

    return result;
}

}
