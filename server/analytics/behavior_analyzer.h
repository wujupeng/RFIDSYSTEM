#pragma once
#include <string>
#include <vector>
#include <memory>

namespace analytics {

struct BehaviorPattern {
    int asset_id;
    std::string epc;
    std::string location;
    int visit_count;
    double avg_stay_duration;
    int max_stay_duration;
    int min_stay_duration;
    std::string most_frequent_time;
};

struct UsageStats {
    int asset_id;
    std::string epc;
    int total_scans;
    int move_count;
    int unique_locations;
    double daily_avg_scans;
    double weekly_avg_scans;
    std::string most_visited_location;
    std::string least_visited_location;
};

class BehaviorAnalyzer {
public:
    static BehaviorAnalyzer& instance();
    
    UsageStats analyzeUsage(int assetId);
    
    UsageStats analyzeUsageByEPC(const std::string& epc);
    
    std::vector<BehaviorPattern> getBehaviorPatterns(int assetId);
    
    std::vector<BehaviorPattern> getBehaviorPatternsByEPC(const std::string& epc);
    
    int getUsageFrequency(int assetId, int64_t startTime = 0, int64_t endTime = 0);
    
    double getAverageStayDuration(int assetId);
    
    std::string getMostVisitedLocation(int assetId);
    
    std::vector<std::pair<std::string, int>> getLocationVisitCounts(int assetId);
    
private:
    BehaviorAnalyzer();
    ~BehaviorAnalyzer() = default;
    BehaviorAnalyzer(const BehaviorAnalyzer&) = delete;
    BehaviorAnalyzer& operator=(const BehaviorAnalyzer&) = delete;
};

} // namespace analytics