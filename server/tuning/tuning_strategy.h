#pragma once

#include "parameter_space.h"
#include <string>
#include <vector>
#include <random>

struct TuningHistoryEntry;

enum class TuningStrategyType {
    Random,
    Grid,
    Gradient,
    Bayesian
};

class TuningStrategy {
public:
    static TuningStrategy& instance();

    void setType(const std::string& type);

    TuningStrategyType getType() const;

    std::unordered_map<std::string, double> suggestNextParameters(
        const std::vector<TuningHistoryEntry>& history,
        const ParameterSpace& space);

    std::vector<std::unordered_map<std::string, double>> generateGridSearch(
        const ParameterSpace& space,
        int points_per_dimension = 5);

    std::unordered_map<std::string, double> randomSearch(
        const ParameterSpace& space);

    std::unordered_map<std::string, double> gradientAscent(
        const std::vector<TuningHistoryEntry>& history,
        const ParameterSpace& space,
        double learning_rate);

private:
    TuningStrategy();

    TuningStrategyType type_;
    std::mt19937 rng_;
};
