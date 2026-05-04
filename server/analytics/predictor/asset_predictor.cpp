#include "asset_predictor.h"
#include "../../core/logger.h"
#include <algorithm>
#include <cmath>

namespace analytics {

AssetPredictor& AssetPredictor::instance() {
    static AssetPredictor instance;
    return instance;
}

RiskScore AssetPredictor::calculateRiskScore(
    double hoursSinceLastSeen,
    double dailyAvgScans,
    int abnormalEvents,
    int locationChanges) {

    RiskScore score;

    // 1. 丢失风险 - 基于最后出现时间
    double timeFactor = hoursSinceLastSeen / 24.0;  // 按天算
    timeFactor = std::min(1.0, timeFactor);  // 上限1.0

    double freqFactor = 1.0 / (dailyAvgScans + 1.0);  // 扫描频率因子

    score.missing_risk = timeFactor * 0.6 + freqFactor * 0.4;
    score.missing_risk = std::min(1.0, score.missing_risk);

    // 2. 闲置风险 - 基于使用频率
    if (dailyAvgScans < 0.1) {
        score.inactivity_risk = 1.0;
    } else if (dailyAvgScans < 0.5) {
        score.inactivity_risk = 0.5;
    } else {
        score.inactivity_risk = 0.1;
    }

    // 3. 异常风险 - 基于异常事件和位置变化
    double abnormalFactor = std::min(1.0, abnormalEvents / 5.0);
    double locationFactor = std::min(1.0, locationChanges / 10.0);

    score.abnormal_risk = abnormalFactor * 0.7 + locationFactor * 0.3;
    score.abnormal_risk = std::min(1.0, score.abnormal_risk);

    // 4. 综合风险
    score.total_risk = score.missing_risk * 0.5 +
                       score.inactivity_risk * 0.2 +
                       score.abnormal_risk * 0.3;
    score.total_risk = std::min(1.0, score.total_risk);

    return score;
}

AssetPrediction AssetPredictor::predictAssetStatus(
    int assetId,
    double hoursSinceLastSeen,
    double dailyAvgScans,
    double weeklyAvgScans,
    int abnormalEvents,
    int locationChanges) {

    AssetPrediction prediction;

    prediction.risk_score = calculateRiskScore(
        hoursSinceLastSeen,
        dailyAvgScans,
        abnormalEvents,
        locationChanges);

    prediction.usage_forecast = forecastUsage(weeklyAvgScans, dailyAvgScans);

    // 预测状态
    if (prediction.risk_score.missing_risk > 0.8) {
        prediction.predicted_status = "AT_RISK_LOST";
    } else if (prediction.risk_score.inactivity_risk > 0.6) {
        prediction.predicted_status = "INACTIVE";
    } else if (prediction.risk_score.abnormal_risk > 0.7) {
        prediction.predicted_status = "ABNORMAL";
    } else if (prediction.usage_forecast > weeklyAvgScans * 1.5) {
        prediction.predicted_status = "OVERUSED";
    } else {
        prediction.predicted_status = "NORMAL";
    }

    spdlog::info("Asset {} prediction: status={}, risk={:.2f}, usage_forecast={:.1f}",
        assetId, prediction.predicted_status, prediction.risk_score.total_risk, prediction.usage_forecast);

    return prediction;
}

double AssetPredictor::forecastUsage(double weeklyAvgScans, double dailyAvgScans) {
    // 加权预测：周权重70%，日权重30%
    return weeklyAvgScans * 0.7 + dailyAvgScans * 0.3;
}

} // namespace analytics