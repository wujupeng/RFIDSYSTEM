#include "bandit_repository.h"
#include "../db/db_pool.h"
#include "../core/logger.h"

namespace repository {

BanditRepository& BanditRepository::instance() {
    static BanditRepository instance;
    return instance;
}

void BanditRepository::insert(
    int asset_id,
    const std::string& action,
    double score,
    double uncertainty,
    double confidence)
{
    try {
        auto conn = DBPool::instance().acquire();
        pqxx::work W(*conn);

        W.exec(
            "INSERT INTO bandit_logs(asset_id, action, score, uncertainty, confidence) VALUES(" +
            W.quote(asset_id) + ", " +
            W.quote(action) + ", " +
            W.quote(score) + ", " +
            W.quote(uncertainty) + ", " +
            W.quote(confidence) + ")"
        );

        W.commit();
        DBPool::instance().release(conn);

        spdlog::debug("Bandit log inserted: asset_id={}, action={}, score={}", asset_id, action, score);
    } catch (const std::exception& e) {
        spdlog::error("Failed to insert bandit log: {}", e.what());
    }
}

std::vector<BanditLogEntry> BanditRepository::getRecentLogs(int limit) {
    std::vector<BanditLogEntry> logs;

    try {
        auto conn = DBPool::instance().acquire();
        pqxx::work W(*conn);

        pqxx::result R = W.exec(
            "SELECT asset_id, action, score, uncertainty, confidence, created_at "
            "FROM bandit_logs ORDER BY created_at DESC LIMIT " + W.quote(limit)
        );

        for (const auto& row : R) {
            BanditLogEntry entry;
            entry.asset_id = row[0].as<int>();
            entry.action = row[1].as<std::string>();
            entry.score = row[2].as<double>();
            entry.uncertainty = row[3].as<double>();
            entry.confidence = row[4].as<double>();
            entry.created_at = row[5].as<std::string>();
            logs.push_back(entry);
        }

        DBPool::instance().release(conn);
    } catch (const std::exception& e) {
        spdlog::error("Failed to get bandit logs: {}", e.what());
    }

    return logs;
}

std::vector<std::tuple<std::string, int>> BanditRepository::getActionDistribution(int hours) {
    std::vector<std::tuple<std::string, int>> distribution;

    try {
        auto conn = DBPool::instance().acquire();
        pqxx::work W(*conn);

        pqxx::result R = W.exec(
            "SELECT action, COUNT(*) as count "
            "FROM bandit_logs "
            "WHERE created_at > NOW() - interval '" + std::to_string(hours) + " hour' "
            "GROUP BY action"
        );

        for (const auto& row : R) {
            distribution.emplace_back(
                row[0].as<std::string>(),
                row[1].as<int>()
            );
        }

        DBPool::instance().release(conn);
    } catch (const std::exception& e) {
        spdlog::error("Failed to get action distribution: {}", e.what());
    }

    return distribution;
}

} // namespace repository
