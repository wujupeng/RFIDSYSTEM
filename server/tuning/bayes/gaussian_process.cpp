#include "gaussian_process.h"
#include <stdexcept>
#include <numeric>
#include <cmath>

GaussianProcess::GaussianProcess()
    : noise_std_(0.01), length_scale_(1.0), signal_variance_(1.0), fitted_(false) {
}

double GaussianProcess::kernel(const std::vector<double>& x1,
                               const std::vector<double>& x2) const {
    double dist_sq = 0.0;
    for (size_t i = 0; i < x1.size(); ++i) {
        double diff = x1[i] - x2[i];
        dist_sq += diff * diff;
    }
    dist_sq /= (length_scale_ * length_scale_);

    return signal_variance_ * std::exp(-0.5 * dist_sq);
}

double GaussianProcess::kernelGrad(const std::vector<double>& x1,
                                  const std::vector<double>& x2,
                                  size_t dim) const {
    if (dim >= x1.size()) return 0.0;

    double diff = x1[dim] - x2[dim];
    double dist_sq = 0.0;
    for (size_t i = 0; i < x1.size(); ++i) {
        double d = x1[i] - x2[i];
        dist_sq += d * d;
    }
    dist_sq /= (length_scale_ * length_scale_);

    double grad = signal_variance_ * std::exp(-0.5 * dist_sq);
    grad *= (diff * diff) / (length_scale_ * length_scale_ * length_scale_ * length_scale_);

    return grad;
}

void GaussianProcess::fit(const std::vector<std::vector<double>>& X,
                          const std::vector<double>& y) {
    if (X.empty() || y.empty()) {
        return;
    }

    X_train_ = X;
    y_train_ = y;

    size_t n = X.size();

    K_.resize(n);
    for (size_t i = 0; i < n; ++i) {
        K_[i].resize(n);
        for (size_t j = 0; j < n; ++j) {
            K_[i][j] = kernel(X[i], X[j]);
            if (i == j) {
                K_[i][j] += noise_std_ * noise_std_;
            }
        }
    }

    K_inv_ = K_;
    for (size_t i = 0; i < n; ++i) {
        double pivot = K_inv_[i][i];
        if (std::abs(pivot) < 1e-10) {
            pivot = 1e-10;
        }
        for (size_t j = i; j < n; ++j) {
            K_inv_[i][j] /= pivot;
        }
        for (size_t k = 0; k < n; ++k) {
            if (k != i) {
                double factor = K_inv_[k][i];
                for (size_t j = i; j < n; ++j) {
                    K_inv_[k][j] -= factor * K_inv_[i][j];
                }
            }
        }
    }

    fitted_ = true;
}

double GaussianProcess::mean(const std::vector<double>& x) const {
    if (!fitted_ || X_train_.empty()) {
        return 0.0;
    }

    size_t n = X_train_.size();
    std::vector<double> k(n);

    for (size_t i = 0; i < n; ++i) {
        k[i] = kernel(x, X_train_[i]);
    }

    double result = 0.0;
    for (size_t i = 0; i < n; ++i) {
        double sum = 0.0;
        for (size_t j = 0; j < n; ++j) {
            sum += K_inv_[i][j] * k[j];
        }
        result += sum * y_train_[i];
    }

    return result;
}

double GaussianProcess::variance(const std::vector<double>& x) const {
    if (!fitted_ || X_train_.empty()) {
        return signal_variance_;
    }

    size_t n = X_train_.size();
    std::vector<double> k(n);

    for (size_t i = 0; i < n; ++i) {
        k[i] = kernel(x, X_train_[i]);
    }

    double k_star = kernel(x, x);

    double kbk = 0.0;
    for (size_t i = 0; i < n; ++i) {
        double sum = 0.0;
        for (size_t j = 0; j < n; ++j) {
            sum += K_inv_[i][j] * k[j];
        }
        kbk += sum * k[i];
    }

    double var = k_star - kbk;
    return std::max(0.0, var);
}

std::pair<double, double> GaussianProcess::predict(
    const std::vector<double>& x) const {
    return {mean(x), variance(x)};
}
