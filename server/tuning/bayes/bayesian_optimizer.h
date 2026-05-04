#pragma once

#include "surrogate_model.h"
#include "acquisition_function.h"
#include "training_sample.h"
#include "../parameter_space.h"
#include <vector>
#include <random>
#include <algorithm>

class BayesianOptimizer {
public:
    BayesianOptimizer();

    void addSample(const TrainingSample& sample);

    void addSample(const std::vector<double>& params, double reward);

    std::vector<double> suggestNext(const ParameterSpace& space);

    void updateModel();

    bool isModelReady() const;

    size_t getNumSamples() const { return dataset_.size(); }

    void setAcquisitionType(AcquisitionType type);

    void setExplorationBonus(double beta) { acq_.setBeta(beta); }

    void setNoiseSmoothing(double noise) { model_.setNoise(noise); }

    double getBestReward() const;

    std::vector<double> getBestParams() const;

private:
    std::vector<double> randomSampleParams(const ParameterSpace& space, size_t n);

    std::vector<double> gridSampleParams(const ParameterSpace& space, size_t n);

    SurrogateModel model_;
    AcquisitionFunction acq_;
    std::vector<TrainingSample> dataset_;
    std::mt19937 rng_;
    bool model_ready_;
};
