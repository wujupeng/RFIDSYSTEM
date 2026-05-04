#include "bayesian_optimizer.h"
#include <algorithm>
#include <cmath>

BayesianOptimizer::BayesianOptimizer()
    : rng_(std::random_device{}()), model_ready_(false) {
    acq_.setType(AcquisitionType::UCB);
    acq_.setBeta(2.0);
}

void BayesianOptimizer::addSample(const TrainingSample& sample) {
    dataset_.push_back(sample);
    model_ready_ = false;
}

void BayesianOptimizer::addSample(const std::vector<double>& params, double reward) {
    TrainingSample sample;
    sample.params = params;
    sample.reward = reward;
    dataset_.push_back(sample);
    model_ready_ = false;
}

void BayesianOptimizer::updateModel() {
    if (dataset_.empty()) {
        return;
    }

    std::vector<TrainingSample> normalized_data = dataset_;
    double min_reward = dataset_[0].reward;
    double max_reward = dataset_[0].reward;

    for (const auto& s : dataset_) {
        min_reward = std::min(min_reward, s.reward);
        max_reward = std::max(max_reward, s.reward);
    }

    double range = max_reward - min_reward;
    if (range < 1e-6) {
        range = 1.0;
    }

    for (auto& s : normalized_data) {
        s.reward = (s.reward - min_reward) / range;
    }

    model_.train(normalized_data);
    model_ready_ = true;
}

bool BayesianOptimizer::isModelReady() const {
    return model_ready_ && !dataset_.empty();
}

std::vector<double> BayesianOptimizer::suggestNext(const ParameterSpace& space) {
    if (!isModelReady()) {
        updateModel();
    }

    if (dataset_.empty()) {
        return randomSampleParams(space, 1)[0];
    }

    const size_t num_candidates = 100;
    auto candidates = randomSampleParams(space, num_candidates);

    double best_reward = getBestReward();
    std::vector<double> best_params;
    double best_score = -1e9;

    for (const auto& candidate : candidates) {
        auto [mean, var] = model_.predictWithUncertainty(candidate);
        double std_dev = std::sqrt(std::max(0.0, var));

        double score = acq_.compute(mean, std_dev);

        if (score > best_score) {
            best_score = score;
            best_params = candidate;
        }
    }

    return best_params;
}

double BayesianOptimizer::getBestReward() const {
    if (dataset_.empty()) {
        return -1e9;
    }

    double best = dataset_[0].reward;
    for (const auto& s : dataset_) {
        if (s.reward > best) {
            best = s.reward;
        }
    }
    return best;
}

std::vector<double> BayesianOptimizer::getBestParams() const {
    if (dataset_.empty()) {
        return {};
    }

    double best_reward = dataset_[0].reward;
    const TrainingSample* best_sample = &dataset_[0];

    for (const auto& s : dataset_) {
        if (s.reward > best_reward) {
            best_reward = s.reward;
            best_sample = &s;
        }
    }

    return best_sample->params;
}

void BayesianOptimizer::setAcquisitionType(AcquisitionType type) {
    acq_.setType(type);
}

std::vector<double> BayesianOptimizer::randomSampleParams(const ParameterSpace& space,
                                                          size_t n) {
    std::vector<double> result;
    auto params = space.getAllParameters();

    for (size_t sample_idx = 0; sample_idx < n; ++sample_idx) {
        std::vector<double> sample;
        std::uniform_real_distribution<double> dist(0.0, 1.0);

        for (const auto& p : params) {
            double random_ratio = dist(rng_);
            double value = p.min_value + random_ratio * (p.max_value - p.min_value);
            value = std::round(value / p.step) * p.step;
            value = std::max(p.min_value, std::min(p.max_value, value));
            sample.push_back(value);
        }

        result.insert(result.end(), sample.begin(), sample.end());
    }

    return result;
}

std::vector<double> BayesianOptimizer::gridSampleParams(const ParameterSpace& space,
                                                        size_t n) {
    auto params = space.getAllParameters();
    std::vector<double> result;

    std::uniform_real_distribution<double> dist(0.0, 1.0);

    for (const auto& p : params) {
        double value = dist(rng_) * (p.max_value - p.min_value) + p.min_value;
        value = std::round(value / p.step) * p.step;
        value = std::max(p.min_value, std::min(p.max_value, value));
        result.push_back(value);
    }

    return result;
}
