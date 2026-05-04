#pragma once

#include <string>

struct RewardResult {
    double accuracy_score;
    double adoption_rate;
    double false_positive_rate;
    double stability_score;
    double total_reward;

    double weighted_accuracy;
    double weighted_adoption;
    double weighted_fp_penalty;
    double weighted_stability;
};

class RewardEvaluator {
public:
    static RewardEvaluator& instance();

    RewardResult evaluate(const std::string& rule_version = "");

    double calculateAccuracy(int lookback_hours = 24);
    double calculateAdoptionRate(int lookback_hours = 24);
    double calculateFalsePositiveRate(int lookback_hours = 24);
    double calculateStabilityScore(int lookback_hours = 24);

    void setWeights(double accuracy, double adoption, double fp_penalty, double stability);

private:
    RewardEvaluator();

    double weight_accuracy_ = 0.4;
    double weight_adoption_ = 0.3;
    double weight_fp_penalty_ = 0.2;
    double weight_stability_ = 0.1;
};
