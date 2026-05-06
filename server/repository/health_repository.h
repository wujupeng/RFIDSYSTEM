#pragma once
#include <string>
#include <vector>
#include <tuple>

namespace repository {

class HealthRepository {
public:
    static HealthRepository& instance();

    void insert(
        const std::string& component,
        const std::string& status,
        int latency_ms = 0,
        const std::string& error_message = ""
    );

    std::vector<std::tuple<std::string, std::string, int, std::string, std::string>> 
    getRecentLogs(int limit = 100);

private:
    HealthRepository() = default;
    HealthRepository(const HealthRepository&) = delete;
    HealthRepository& operator=(const HealthRepository&) = delete;
};

} // namespace repository
