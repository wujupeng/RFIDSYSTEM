#include "linucb_algorithm.h"

namespace bandit {

LinUCBAlgorithm::LinUCBAlgorithm(int feature_dim, double alpha)
    : feature_dim_(feature_dim),
      alpha_(alpha),
      epsilon_(0.1),
      epsilon_min_(0.02),
      total_samples_(0),
      rng_(std::random_device{}()) {
    initializeArms();
}

void LinUCBAlgorithm::setEpsilon(double eps) {
    epsilon_ = eps;
}

void LinUCBAlgorithm::setAlpha(double alpha) {
    alpha_ = alpha;
}

double LinUCBAlgorithm::getCurrentEpsilon() const {
    double decay = std::max(0.0, 1.0 - total_samples_ / 10000.0);
    return std::max(epsilon_min_, epsilon_ * decay);
}

Action LinUCBAlgorithm::selectAction(const ContextFeatures& context) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    Eigen::VectorXd x = context.toVector();
    
    // epsilon-greedy 探索
    if (std::uniform_real_distribution<double>(0, 1)(rng_) < getCurrentEpsilon()) {
        return selectRandomAction();
    }
    
    double best_score = -1e9;
    Action best_action;
    
    for (auto& arm : arms_) {
        try {
            Eigen::VectorXd theta = arm.A.inverse() * arm.b;
            
            double exploit = theta.transpose() * x;
            double explore = alpha_ * std::sqrt(x.transpose() * arm.A.inverse() * x);
            double score = exploit + explore;
            
            if (score > best_score) {
                best_score = score;
                best_action = arm.action;
            }
        } catch (...) {
            // 矩阵奇异时使用默认动作
            continue;
        }
    }
    
    return best_action;
}

void LinUCBAlgorithm::update(const ContextFeatures& context, const Action& action, double reward) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    Eigen::VectorXd x = context.toVector();
    
    for (auto& arm : arms_) {
        if (arm.action.type == action.type) {
            arm.A += x * x.transpose();
            arm.b += reward * x;
            arm.sample_count++;
            total_samples_.fetch_add(1);
            break;
        }
    }
}

double LinUCBAlgorithm::predictReward(const ContextFeatures& context, const Action& action) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    Eigen::VectorXd x = context.toVector();
    
    for (const auto& arm : arms_) {
        if (arm.action.type == action.type) {
            try {
                Eigen::VectorXd theta = arm.A.inverse() * arm.b;
                return theta.transpose() * x;
            } catch (...) {
                return 0.0;
            }
        }
    }
    return 0.0;
}

std::vector<std::pair<Action, double>> LinUCBAlgorithm::getAllPredictions(const ContextFeatures& context) const {
    std::vector<std::pair<Action, double>> results;
    Eigen::VectorXd x = context.toVector();
    
    for (const auto& arm : arms_) {
        try {
            Eigen::VectorXd theta = arm.A.inverse() * arm.b;
            double reward = theta.transpose() * x;
            results.emplace_back(arm.action, reward);
        } catch (...) {
            results.emplace_back(arm.action, 0.0);
        }
    }
    
    return results;
}

std::vector<ArmModel> LinUCBAlgorithm::getModels() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return arms_;
}

void LinUCBAlgorithm::loadModels(const std::vector<ArmModel>& models) {
    std::lock_guard<std::mutex> lock(mutex_);
    arms_ = models;
}

} // namespace bandit