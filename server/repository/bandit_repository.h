#pragma once
#include <string>
#include <vector>

namespace repository {

struct BanditLogEntry {
    int asset_id;
    std::string action;
    double score;
    double uncertainty;
    double confidence;
    std::string created_at;
};

class BanditRepository {
public:
    static BanditRepository& instance();

    void insert(
        int asset_id,
        const std::string& action,
        double score,
        double uncertainty,
        double confidence
    );

    std::vector<BanditLogEntry> getRecentLogs(int limit = 100);
    std::vector<std::tuple<std::string, int>> getActionDistribution(int hours = 1);

private:
    BanditRepository() = default;
    BanditRepository(const BanditRepository&) = delete;
    BanditRepository& operator=(const BanditRepository&) = delete;
};

} // namespace repository
