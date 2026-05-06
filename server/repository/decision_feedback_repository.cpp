#include "decision_feedback_repository.h"
#include "../db/db_pool.h"
#include "../core/logger.h"

namespace repository {

DecisionFeedbackRepository& DecisionFeedbackRepository::instance() {
    static DecisionFeedbackRepository instance;
    return instance;
}

void DecisionFeedbackRepository::insert(int decision_id, int asset_id, bool executed, bool ignored, int user_id) {
    try {
        auto conn = DBPool::instance().acquire();
        pqxx::work W(*conn);

        W.exec(
            "INSERT INTO decision_feedback(decision_id, asset_id, executed, ignored, user_id) VALUES(" +
            W.quote(decision_id) + ", " +
            W.quote(asset_id) + ", " +
            W.quote(executed) + ", " +
            W.quote(ignored) + ", " +
            W.quote(user_id) + ")"
        );

        W.commit();
        DBPool::instance().release(conn);

        spdlog::debug("Decision feedback inserted: decision_id={}, asset_id={}, executed={}", decision_id, asset_id, executed);
    } catch (const std::exception& e) {
        spdlog::error("Failed to insert decision feedback: {}", e.what());
    }
}

double DecisionFeedbackRepository::getAdoptionRate(int hours) {
    try {
        auto conn = DBPool::instance().acquire();
        pqxx::work W(*conn);

        pqxx::result R = W.exec(
            "SELECT "
            "  SUM(CASE WHEN executed THEN 1 ELSE 0 END) as executed_count, "
            "  COUNT(*) as total_count "
            "FROM decision_feedback "
            "WHERE feedback_time > NOW() - interval '" + std::to_string(hours) + " hour'"
        );

        DBPool::instance().release(conn);

        if (!R.empty() && R[0][1].as<int>() > 0) {
            return static_cast<double>(R[0][0].as<int>()) / R[0][1].as<int>();
        }
    } catch (const std::exception& e) {
        spdlog::error("Failed to get adoption rate: {}", e.what());
    }

    return 0.0;
}

int DecisionFeedbackRepository::getTotalFeedbacks(int hours) {
    try {
        auto conn = DBPool::instance().acquire();
        pqxx::work W(*conn);

        pqxx::result R = W.exec(
            "SELECT COUNT(*) FROM decision_feedback "
            "WHERE feedback_time > NOW() - interval '" + std::to_string(hours) + " hour'"
        );

        DBPool::instance().release(conn);

        if (!R.empty()) {
            return R[0][0].as<int>();
        }
    } catch (const std::exception& e) {
        spdlog::error("Failed to get total feedbacks: {}", e.what());
    }

    return 0;
}

int DecisionFeedbackRepository::getExecutedCount(int hours) {
    try {
        auto conn = DBPool::instance().acquire();
        pqxx::work W(*conn);

        pqxx::result R = W.exec(
            "SELECT COUNT(*) FROM decision_feedback "
            "WHERE executed = true AND feedback_time > NOW() - interval '" + std::to_string(hours) + " hour'"
        );

        DBPool::instance().release(conn);

        if (!R.empty()) {
            return R[0][0].as<int>();
        }
    } catch (const std::exception& e) {
        spdlog::error("Failed to get executed count: {}", e.what());
    }

    return 0;
}

} // namespace repository
