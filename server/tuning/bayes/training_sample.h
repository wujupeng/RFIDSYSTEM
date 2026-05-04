#pragma once

#include <vector>

struct TrainingSample {
    std::vector<double> params;
    double reward;
    double timestamp;
    bool is_noisy;

    TrainingSample() : reward(0.0), timestamp(0.0), is_noisy(false) {}

    TrainingSample(const std::vector<double>& p, double r)
        : params(p), reward(r), timestamp(0.0), is_noisy(false) {}
};
