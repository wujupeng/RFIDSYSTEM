#include "health_repository.h"
#include "../db/db_pool.h"
#include "../core/logger.h"

namespace repository {

HealthRepository& HealthRepository::instance() {
    static HealthRepository instance;
    return instance;
}

void HealthRepository::insert(
    const std::string& component,
    const std::string& status,
    int latency_ms,
    const std::string& error_message)
{
    try {
        auto conn = DBPool::instance().acquire();
        pqxx::work W(*conn);

        W.exec(
            "INSERT INTO health_logs(component, status, latency_ms, error_message) VALUES(" +
            W.quote(component) + ", " +
            W.quote(status) + ", " +
            W.quote(latency_ms) + ", " +
            W.quote(error_message) + ")"
        );

        W.commit();
        DBPool::instance().release(conn);

        spdlog::debug("Health log inserted: component={}, status={}", component, status);
    } catch (const std::exception& e) {
        spdlog::error("Failed to insert health log: {}", e.what());
    }
}

std::vector<std::tuple<std::string, std::string, int, std::string, std::string>> 
HealthRepository::getRecentLogs(int limit) {
    std::vector<std::tuple<std::string, std::string, int, std::string, std::string>> logs;

    try {
        auto conn = DBPool::instance().acquire();
        pqxx::work W(*conn);

        pqxx::result R = W.exec(
            "SELECT component, status, latency_ms, error_message, created_at "
            "FROM health_logs ORDER BY created_at DESC LIMIT " + W.quote(limit)
        );

        for (const auto& row : R) {
            logs.emplace_back(
                row[0].as<std::string>(),
                row[1].as<std::string>(),
                row[2].as<int>(),
                row[3].as<std::string>(),
                row[4].as<std::string>()
            );
        }

        DBPool::instance().release(conn);
    } catch (const std::exception& e) {
        spdlog::error("Failed to get health logs: {}", e.what());
    }

    return logs;
}

} // namespace repository
