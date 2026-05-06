#include "metrics_repository.h"
#include "../db/db_pool.h"
#include "../core/logger.h"

namespace repository {

MetricsRepository& MetricsRepository::instance() {
    static MetricsRepository instance;
    return instance;
}

void MetricsRepository::insert(const std::string& name, double value) {
    insert(name, value, nlohmann::json::object());
}

void MetricsRepository::insert(const std::string& name, double value, const nlohmann::json& tags) {
    try {
        auto conn = DBPool::instance().acquire();
        pqxx::work W(*conn);

        std::string tagsJson = tags.dump();

        W.exec(
            "INSERT INTO system_metrics(metric_name, metric_value, tags) VALUES(" +
            W.quote(name) + ", " +
            W.quote(value) + ", " +
            W.quote(tagsJson) + ")"
        );

        W.commit();
        DBPool::instance().release(conn);

        spdlog::debug("Metric inserted: name={}, value={}", name, value);
    } catch (const std::exception& e) {
        spdlog::error("Failed to insert metric: {}", e.what());
    }
}

double MetricsRepository::getLatestValue(const std::string& name) {
    try {
        auto conn = DBPool::instance().acquire();
        pqxx::work W(*conn);

        pqxx::result R = W.exec(
            "SELECT metric_value FROM system_metrics "
            "WHERE metric_name = " + W.quote(name) +
            " ORDER BY created_at DESC LIMIT 1"
        );

        DBPool::instance().release(conn);

        if (!R.empty()) {
            return R[0][0].as<double>();
        }
    } catch (const std::exception& e) {
        spdlog::error("Failed to get metric: {}", e.what());
    }

    return 0.0;
}

std::vector<std::tuple<std::string, double, std::string>> MetricsRepository::getRecentMetrics(const std::string& name, int limit) {
    std::vector<std::tuple<std::string, double, std::string>> metrics;

    try {
        auto conn = DBPool::instance().acquire();
        pqxx::work W(*conn);

        pqxx::result R = W.exec(
            "SELECT metric_value, created_at FROM system_metrics "
            "WHERE metric_name = " + W.quote(name) +
            " ORDER BY created_at DESC LIMIT " + W.quote(limit)
        );

        for (const auto& row : R) {
            metrics.emplace_back(
                name,
                row[0].as<double>(),
                row[1].as<std::string>()
            );
        }

        DBPool::instance().release(conn);
    } catch (const std::exception& e) {
        spdlog::error("Failed to get metrics: {}", e.what());
    }

    return metrics;
}

} // namespace repository
