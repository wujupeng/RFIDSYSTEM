#pragma once

#include "training_sample.h"
#include "gaussian_process.h"
#include <vector>

class SurrogateModel {
public:
    SurrogateModel();

    void train(const std::vector<TrainingSample>& data);

    double predict(const std::vector<double>& params);

    double uncertainty(const std::vector<double>& params);

    std::pair<double, double> predictWithUncertainty(const std::vector<double>& params);

    bool isTrained() const { return trained_; }

    void setNoise(double noise);
    void setLengthScale(double scale);
    void setSignalVariance(double variance);

private:
    GaussianProcess gp_;
    bool trained_;
    std::vector<TrainingSample> data_;
    std::vector<std::vector<double>> X_;
    std::vector<double> y_;
};
