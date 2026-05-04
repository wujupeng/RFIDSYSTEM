#include "auto_tuner.h"
#include <spdlog/spdlog.h>
#include <chrono>

AutoTuner& AutoTuner::instance() {
    static AutoTuner instance;
    return instance;
}

AutoTuner::AutoTuner()
    : best_reward_(-1e9) {
    space_.initializeDefaults();
}

void AutoTuner::initialize() {
    space_.initializeDefaults();
    history_.clear();
    best_reward_ = -1e9;
    best_params_.clear();
}

void AutoTuner::runIteration() {
    std::unordered_map<std::string, double> params;

    if (bayes_optimizer_.isModelReady()) {
        params = bayes_optimizer_.suggestNext(space_);
    } else {
        params = strategy_.suggestNextParameters(history_, space_);
    }

    space_.restore(params);

    RewardResult reward = evaluator_.evaluate();

    TuningHistoryEntry entry;
    entry.iteration = static_cast<int>(history_.size());
    entry.parameters = space_.snapshot();
    entry.total_reward = reward.total_reward;
    entry.improvement = reward.total_reward - best_reward_;

    history_.push_back(entry);

    bayes_optimizer_.addSample(entry.parameters, reward.total_reward);
    bayes_optimizer_.updateModel();

    if (reward.total_reward > best_reward_) {
        best_reward_ = reward.total_reward;
        best_params_ = space_.snapshot();

        spdlog::info("[AutoTuner] NEW BEST: reward={:.4f}", best_reward_);
    }
}

TuningResult AutoTuner::start(int iterations, const std::string& strategy) {
    TuningResult result;
    result.success = false;
    result.best_reward = -1e9;

    auto start_time = std::chrono::steady_clock::now();

    strategy_.setType(strategy);
    initialize();

    if (strategy == "bayesian") {
        bayes_optimizer_.setExplorationBonus(beta_);
        bayes_optimizer_.setNoiseSmoothing(noise_smoothing_);
    }

    spdlog::info("[AutoTuner] Starting tuning with strategy: {}", strategy);
    spdlog::info("[AutoTuner] Parameters: {}", space_.snapshot().size());

    if (strategy == "bayesian") {
        for (int i = 0; i < 5; ++i) {
            auto params = strategy_.suggestNextParameters(history_, space_);
            space_.restore(params);
            RewardResult reward = evaluator_.evaluate();
            bayes_optimizer_.addSample(params, reward.total_reward);

            if (reward.total_reward > best_reward_) {
                best_reward_ = reward.total_reward;
                best_params_ = space_.snapshot();
            }
        }
        bayes_optimizer_.updateModel();
    }

    for (int i = 0; i < iterations; ++i) {
        runIteration();

        if ((i + 1) % 10 == 0) {
            spdlog::info("[AutoTuner] Iteration {}/{}, current best: {:.4f}",
                        i + 1, iterations, best_reward_);
        }
    }

    auto end_time = std::chrono::steady_clock::now();
    double runtime = std::chrono::duration<double>(end_time - start_time).count();

    result.success = true;
    result.message = "Tuning completed successfully";
    result.best_reward = best_reward_;
    result.best_parameters = best_params_;
    result.iterations_run = iterations;
    result.runtime_seconds = runtime;

    best_result_ = result;

    spdlog::info("[AutoTuner] Tuning finished. Best reward: {:.4f}, Runtime: {:.2f}s",
                result.best_reward, result.runtime_seconds);

    return result;
}

void AutoTuner::setLearningRate(double lr) {
    learning_rate_ = lr;
}

void AutoTuner::setExplorationRate(double rate) {
    exploration_rate_ = rate;
}

void AutoTuner::setBeta(double beta) {
    beta_ = beta;
}

void AutoTuner::setNoiseSmoothing(double noise) {
    noise_smoothing_ = noise;
}

std::vector<TuningHistoryEntry> AutoTuner::getHistory() const {
    return history_;
}

TuningResult AutoTuner::getBestResult() const {
    return best_result_;
}

void AutoTuner::saveBestParameters() {
    if (!best_params_.empty()) {
        space_.restore(best_params_);
        spdlog::info("[AutoTuner] Best parameters restored to parameter space");
    }
}

void AutoTuner::rollback() {
    space_.resetToDefaults();
    spdlog::info("[AutoTuner] Parameters rolled back to defaults");
}
