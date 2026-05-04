#pragma once

#include <vector>
#include <cmath>
#include <algorithm>

class GaussianProcess {
public:
    GaussianProcess();

    void fit(const std::vector<std::vector<double>>& X,
             const std::vector<double>& y);

    double mean(const std::vector<double>& x) const;

    double variance(const std::vector<double>& x) const;

    std::pair<double, double> predict(const std::vector<double>& x) const;

    void setNoise(double noise) { noise_std_ = noise; }

    void setLengthScale(double scale) { length_scale_ = scale; }

    void setSignalVariance(double variance) { signal_variance_ = variance; }

private:
    double kernel(const std::vector<double>& x1,
                  const std::vector<double>& x2) const;

    double kernelGrad(const std::vector<double>& x1,
                      const std::vector<double>& x2,
                      size_t dim) const;

    std::vector<std::vector<double>> X_train_;
    std::vector<double> y_train_;
    std::vector<std::vector<double>> K_;
    std::vector<std::vector<double>> K_inv_;
    bool fitted_;

    double noise_std_;
    double length_scale_;
    double signal_variance_;
};
