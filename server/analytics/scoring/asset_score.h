#pragma once
#include <string>

namespace analytics {

enum class RiskLevel {
    LOW,
    MEDIUM,
    HIGH,
    CRITICAL
};

struct AssetScore {
    double health_score;       // 健康度 0~100
    double risk_score;         // 风险 0~1
    RiskLevel risk_level;      // 风险等级
    std::string risk_summary;  // 风险摘要
};

class AssetScoring {
public:
    static AssetScoring& instance();

    AssetScore calculateAssetScore(
        double missingRisk,
        double inactivityRisk,
        double abnormalRisk,
        double dailyAvgScans,
        int repairCount);

    std::string getRiskLevelString(RiskLevel level);

private:
    AssetScoring() = default;
    AssetScoring(const AssetScoring&) = delete;
    AssetScoring& operator=(const AssetScoring&) = delete;

    RiskLevel determineRiskLevel(double healthScore);
    std::string generateRiskSummary(
        double missingRisk,
        double inactivityRisk,
        double abnormalRisk,
        int repairCount);
};

} // namespace analytics