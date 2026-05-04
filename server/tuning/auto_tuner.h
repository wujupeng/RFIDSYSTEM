#pragma once

#include "parameter_space.h"
#include "reward_evaluator.h"
#include "tuning_strategy.h"
#include "bayes/bayesian_optimizer.h"
#include "bayes/training_sample.h"
#include <string>
#include <unordered_map>
#include <vector>

struct TuningHistoryEntry {
    int iteration;
    std::unordered_map<std::string, double> parameters;
    double total_reward;
    double improvement;
};

struct TuningResult {
    bool success;
    std::string message;
    double best_reward;
    std::unordered_map<std::string, double> best_parameters;
    int iterations_run;
    double runtime_seconds;
};

class AutoTuner {
public:
    static AutoTuner& instance();

    void initialize();

    void runIteration();

    TuningResult start(int iterations, const std::string& strategy = "random");

    void setLearningRate(double lr);

    void setExplorationRate(double rate);

    void setBeta(double beta);

    void setNoiseSmoothing(double noise);

    std::vector<TuningHistoryEntry> getHistory() const;

    TuningResult getBestResult() const;

    void saveBestParameters();

    void rollback();

private:
    AutoTuner();

    ParameterSpace space_;
    RewardEvaluator evaluator_;
    TuningStrategy strategy_;
    BayesianOptimizer bayes_optimizer_;

    double learning_rate_ = 0.1;
    double exploration_rate_ = 0.2;
    double beta_ = 2.0;
    double noise_smoothing_ = 0.1;

    TuningResult best_result_;
    std::vector<TuningHistoryEntry> history_;

    double best_reward_;
    std::unordered_map<std::string, double> best_params_;
};
