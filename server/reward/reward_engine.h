#pragma once
#include <string>
#include "../models/decision_outcome.h"

namespace v33 {

struct RewardResult {
    double reward;
    double true_positive;
    double false_positive;
    double false_negative;
    double true_negative;
    std::string reward_type;
};

class RewardEngine {
public:
    static RewardEngine& instance();

    double computeReward(
        int decision_id,
        const DecisionOutcome& outcome,
        const std::string& action
    );

    RewardResult computeDetailedReward(
        int decision_id,
        const DecisionOutcome& outcome,
        const std::string& action
    );

    static constexpr double REWARD_TRUE_POSITIVE = 0.8;
    static constexpr double REWARD_TRUE_NEGATIVE = 0.1;
    static constexpr double PENALTY_FALSE_POSITIVE = -0.2;
    static constexpr double PENALTY_FALSE_NEGATIVE = -1.0;

private:
    RewardEngine() = default;
};

}
