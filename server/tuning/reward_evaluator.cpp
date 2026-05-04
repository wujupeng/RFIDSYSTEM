#include "reward_evaluator.h"
#include "../../repository/decision_repository.h"
#include "../../db/db_pool.h"
#include <pqxx/pqxx>
#include <spdlog/spdlog.h>

RewardEvaluator& RewardEvaluator::instance() {
    static RewardEvaluator instance;
    return instance;
}

RewardEvaluator::RewardEvaluator() = default;

RewardResult RewardEvaluator::evaluate(const std::string& rule_version) {
    RewardResult result;

    result.accuracy_score = calculateAccuracy();
    result.adoption_rate = calculateAdoptionRate();
    result.false_positive_rate = calculateFalsePositiveRate();
    result.stability_score = calculateStabilityScore();

    result.weighted_accuracy = weight_accuracy_ * result.accuracy_score;
    result.weighted_adoption = weight_adoption_ * result.adoption_rate;
    result.weighted_fp_penalty = weight_fp_penalty_ * (1.0 - result.false_positive_rate);
    result.weighted_stability = weight_stability_ * result.stability_score;

    result.total_reward = result.weighted_accuracy
                        + result.weighted_adoption
                        + result.weighted_fp_penalty
                        + result.weighted_stability;

    return result;
}

double RewardEvaluator::calculateAccuracy(int lookback_hours) {
    auto conn = DBPool::instance().acquire();
    double accuracy = 0.0;

    try {
        pqxx::work txn(*conn);

        auto res = txn.exec_params(
            R"(
            SELECT
                COUNT(*) FILTER (WHERE d.executed = TRUE) as executed_count,
                COUNT(*) FILTER (WHERE d.ignored = TRUE) as ignored_count,
                COUNT(*) as total_count
            FROM decisions d
            WHERE d.created_at > NOW() - ($1 * INTERVAL '1 hour')
            )",
            lookback_hours
        );

        if (!res.empty()) {
            int executed = res[0]["executed_count"].as<int>();
            int ignored = res[0]["ignored_count"].as<int>();
            int total = res[0]["total_count"].as<int>();

            if (total > 0) {
                accuracy = static_cast<double>(executed) / total;
            }
        }

    } catch (const std::exception& e) {
        spdlog::error("Calculate accuracy failed: {}", e.what());
    }

    DBPool::instance().release(conn);
    return accuracy;
}

double RewardEvaluator::calculateAdoptionRate(int lookback_hours) {
    return DecisionRepository::instance().getAdoptionRate(lookback_hours);
}

double RewardEvaluator::calculateFalsePositiveRate(int lookback_hours) {
    auto conn = DBPool::instance().acquire();
    double fp_rate = 0.0;

    try {
        pqxx::work txn(*conn);

        auto res = txn.exec_params(
            R"(
            SELECT
                COUNT(*) FILTER (WHERE d.ignored = TRUE) as ignored_count,
                COUNT(*) as total_count
            FROM decisions d
            WHERE d.created_at > NOW() - ($1 * INTERVAL '1 hour')
            )",
            lookback_hours
        );

        if (!res.empty()) {
            int ignored = res[0]["ignored_count"].as<int>();
            int total = res[0]["total_count"].as<int>();

            if (total > 0) {
                fp_rate = static_cast<double>(ignored) / total;
            }
        }

    } catch (const std::exception& e) {
        spdlog::error("Calculate FPR failed: {}", e.what());
    }

    DBPool::instance().release(conn);
    return fp_rate;
}

double RewardEvaluator::calculateStabilityScore(int lookback_hours) {
    auto conn = DBPool::instance().acquire();
    double stability = 0.0;

    try {
        pqxx::work txn(*conn);

        auto res = txn.exec_params(
            R"(
            SELECT COUNT(DISTINCT asset_id) as unique_assets,
                   COUNT(*) as total_decisions
            FROM decisions
            WHERE created_at > NOW() - ($1 * INTERVAL '1 hour')
            )",
            lookback_hours
        );

        if (!res.empty()) {
            int unique = res[0]["unique_assets"].as<int>();
            int total = res[0]["total_decisions"].as<int>();

            if (total > 0) {
                stability = static_cast<double>(unique) / total;
            }
        }

    } catch (const std::exception& e) {
        spdlog::error("Calculate stability failed: {}", e.what());
    }

    DBPool::instance().release(conn);
    return stability;
}

void RewardEvaluator::setWeights(double accuracy, double adoption, double fp_penalty, double stability) {
    weight_accuracy_ = accuracy;
    weight_adoption_ = adoption;
    weight_fp_penalty_ = fp_penalty;
    weight_stability_ = stability;
}
