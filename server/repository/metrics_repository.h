#pragma once
#include <string>
#include <nlohmann/json.hpp>

namespace repository {

class MetricsRepository {
public:
    static MetricsRepository& instance();

    void insert(const std::string& name, double value);
    void insert(const std::string& name, double value, const nlohmann::json& tags);

    double getLatestValue(const std::string& name);
    std::vector<std::tuple<std::string, double, std::string>> getRecentMetrics(const std::string& name, int limit = 60);

private:
    MetricsRepository() = default;
    MetricsRepository(const MetricsRepository&) = delete;
    MetricsRepository& operator=(const MetricsRepository&) = delete;
};

} // namespace repository
