#include "tuning_strategy.h"
#include <random>
#include <algorithm>
#include <cmath>

TuningStrategy& TuningStrategy::instance() {
    static TuningStrategy instance;
    return instance;
}

TuningStrategy::TuningStrategy()
    : type_(TuningStrategyType::Random), rng_(std::random_device{}()) {
}

void TuningStrategy::setType(const std::string& type) {
    if (type == "random") {
        type_ = TuningStrategyType::Random;
    } else if (type == "grid") {
        type_ = TuningStrategyType::Grid;
    } else if (type == "gradient") {
        type_ = TuningStrategyType::Gradient;
    } else if (type == "bayesian") {
        type_ = TuningStrategyType::Bayesian;
    } else {
        type_ = TuningStrategyType::Random;
    }
}

TuningStrategyType TuningStrategy::getType() const {
    return type_;
}

std::unordered_map<std::string, double> TuningStrategy::suggestNextParameters(
    const std::vector<TuningHistoryEntry>& history,
    const ParameterSpace& space) {

    switch (type_) {
        case TuningStrategyType::Random:
            return randomSearch(space);
        case TuningStrategyType::Gradient:
            return gradientAscent(history, space, 0.1);
        default:
            return randomSearch(space);
    }
}

std::vector<std::unordered_map<std::string, double>> TuningStrategy::generateGridSearch(
    const ParameterSpace& space,
    int points_per_dimension) {

    std::vector<std::unordered_map<std::string, double>> results;
    auto params = space.getAllParameters();

    std::vector<std::vector<double>> values(params.size());
    for (size_t i = 0; i < params.size(); ++i) {
        double min_val = params[i].min_value;
        double max_val = params[i].max_value;
        double step = (max_val - min_val) / (points_per_dimension - 1);

        for (int j = 0; j < points_per_dimension; ++j) {
            values[i].push_back(min_val + step * j);
        }
    }

    std::vector<size_t> indices(params.size(), 0);
    bool done = false;
    while (!done) {
        std::unordered_map<std::string, double> combo;
        for (size_t i = 0; i < params.size(); ++i) {
            combo[params[i].name] = values[i][indices[i]];
        }
        results.push_back(combo);

        indices[0]++;
        for (size_t i = 0; i < params.size() - 1; ++i) {
            if (indices[i] >= values[i].size()) {
                indices[i] = 0;
                indices[i + 1]++;
            }
        }
        if (indices[params.size() - 1] >= values[params.size() - 1].size()) {
            done = true;
        }
    }

    return results;
}

std::unordered_map<std::string, double> TuningStrategy::randomSearch(
    const ParameterSpace& space) {

    std::unordered_map<std::string, double> result;
    auto params = space.getAllParameters();

    std::uniform_real_distribution<double> dist(0.0, 1.0);

    for (const auto& p : params) {
        double random_ratio = dist(rng_);
        double new_value = p.min_value + random_ratio * (p.max_value - p.min_value);
        new_value = std::round(new_value / p.step) * p.step;
        new_value = std::max(p.min_value, std::min(p.max_value, new_value));
        result[p.name] = new_value;
    }

    return result;
}

std::unordered_map<std::string, double> TuningStrategy::gradientAscent(
    const std::vector<TuningHistoryEntry>& history,
    const ParameterSpace& space,
    double learning_rate) {

    if (history.size() < 2) {
        return randomSearch(space);
    }

    std::unordered_map<std::string, double> result;
    auto params = space.getAllParameters();

    for (const auto& p : params) {
        double gradient = 0.0;

        if (history.size() >= 2) {
            double recent = history.back().parameters.count(p.name)
                ? history.back().parameters.at(p.name) : p.current_value;
            double previous = history[history.size() - 2].parameters.count(p.name)
                ? history[history.size() - 2].parameters.at(p.name) : p.current_value;

            double reward_recent = history.back().reward.total_reward;
            double reward_previous = history[history.size() - 2].reward.total_reward;

            if (std::abs(recent - previous) > 1e-6) {
                gradient = (reward_recent - reward_previous) / (recent - previous);
            }
        }

        double new_value = p.current_value + learning_rate * gradient;
        new_value = std::max(p.min_value, std::min(p.max_value, new_value));
        result[p.name] = new_value;
    }

    return result;
}
