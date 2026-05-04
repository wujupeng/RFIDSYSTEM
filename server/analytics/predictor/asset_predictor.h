#pragma once
#include <string>

namespace analytics {

struct RiskScore {
    double missing_risk;      // 丢失风险 0~1
    double inactivity_risk;   // 长期未使用风险 0~1
    double abnormal_risk;     // 行为异常风险 0~1
    double total_risk;        // 综合风险
};

struct AssetPrediction {
    std::string predicted_status;  // 预测状态
    double usage_forecast;         // 使用量预测
    RiskScore risk_score;
};

class AssetPredictor {
public:
    static AssetPredictor& instance();

    RiskScore calculateRiskScore(
        double hoursSinceLastSeen,
        double dailyAvgScans,
        int abnormalEvents,
        int locationChanges);

    AssetPrediction predictAssetStatus(
        int assetId,
        double hoursSinceLastSeen,
        double dailyAvgScans,
        double weeklyAvgScans,
        int abnormalEvents,
        int locationChanges);

    double forecastUsage(double weeklyAvgScans, double dailyAvgScans);

private:
    AssetPredictor() = default;
    AssetPredictor(const AssetPredictor&) = delete;
    AssetPredictor& operator=(const AssetPredictor&) = delete;
};

} // namespace analytics