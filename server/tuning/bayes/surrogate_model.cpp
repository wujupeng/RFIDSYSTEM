#include "surrogate_model.h"
#include <algorithm>

SurrogateModel::SurrogateModel() : trained_(false) {
    gp_.setNoise(0.1);
    gp_.setLengthScale(1.0);
    gp_.setSignalVariance(1.0);
}

void SurrogateModel::train(const std::vector<TrainingSample>& data) {
    if (data.empty()) {
        return;
    }

    data_ = data;
    X_.clear();
    y_.clear();

    for (const auto& sample : data) {
        X_.push_back(sample.params);
        y_.push_back(sample.reward);
    }

    std::vector<double> y_normalized = y_;
    double y_mean = 0.0;
    double y_std = 0.0;

    for (double val : y_normalized) {
        y_mean += val;
    }
    y_mean /= y_normalized.size();

    for (double val : y_normalized) {
        y_std += (val - y_mean) * (val - y_mean);
    }
    y_std = std::sqrt(y_std / y_normalized.size());

    if (y_std > 1e-6) {
        for (auto& val : y_normalized) {
            val = (val - y_mean) / y_std;
        }
    }

    gp_.fit(X_, y_normalized);
    trained_ = true;
}

double SurrogateModel::predict(const std::vector<double>& params) {
    if (!trained_) {
        return 0.0;
    }

    auto [mean, var] = gp_.predict(params);
    return mean;
}

double SurrogateModel::uncertainty(const std::vector<double>& params) {
    if (!trained_) {
        return 1.0;
    }

    auto [mean, var] = gp_.predict(params);
    return std::sqrt(std::max(0.0, var));
}

std::pair<double, double> SurrogateModel::predictWithUncertainty(
    const std::vector<double>& params) {
    if (!trained_) {
        return {0.0, 1.0};
    }

    return gp_.predict(params);
}

void SurrogateModel::setNoise(double noise) {
    gp_.setNoise(noise);
}

void SurrogateModel::setLengthScale(double scale) {
    gp_.setLengthScale(scale);
}

void SurrogateModel::setSignalVariance(double variance) {
    gp_.setSignalVariance(variance);
}
