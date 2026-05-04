#include "asset_score.h"
#include "../../core/logger.h"
#include <sstream>
#include <algorithm>

namespace analytics {

AssetScoring& AssetScoring::instance() {
    static AssetScoring instance;
    return instance;
}

AssetScore AssetScoring::calculateAssetScore(
    double missingRisk,
    double inactivityRisk,
    double abnormalRisk,
    double dailyAvgScans,
    int repairCount) {

    AssetScore score;

    // 基础分 100
    double baseScore = 100.0;

    // 扣分项
    baseScore -= missingRisk * 40;       // 丢失风险扣最多40分
    baseScore -= abnormalRisk * 30;      // 异常风险扣最多30分
    baseScore -= inactivityRisk * 20;    // 闲置风险扣最多20分
    baseScore -= std::min(10.0, repairCount * 2.0);  // 维修记录每笔扣2分，最多10分

    // 加分项
    if (dailyAvgScans > 10) {
        baseScore += 5.0;  // 高频使用加5分
    }

    // 确保分数在 0~100 之间
    score.health_score = std::max(0.0, std::min(100.0, baseScore));

    // 综合风险评分
    score.risk_score = missingRisk * 0.5 +
                      abnormalRisk * 0.3 +
                      inactivityRisk * 0.2;
    score.risk_score = std::min(1.0, score.risk_score);

    // 确定风险等级
    score.risk_level = determineRiskLevel(score.health_score);

    // 生成风险摘要
    score.risk_summary = generateRiskSummary(
        missingRisk,
        inactivityRisk,
        abnormalRisk,
        repairCount);

    spdlog::info("Asset score: health={:.1f}, risk={:.2f}, level={}",
        score.health_score, score.risk_score, getRiskLevelString(score.risk_level));

    return score;
}

RiskLevel AssetScoring::determineRiskLevel(double healthScore) {
    if (healthScore < 60) {
        return RiskLevel::CRITICAL;
    } else if (healthScore < 80) {
        return RiskLevel::HIGH;
    } else if (healthScore < 90) {
        return RiskLevel::MEDIUM;
    } else {
        return RiskLevel::LOW;
    }
}

std::string AssetScoring::getRiskLevelString(RiskLevel level) {
    switch (level) {
        case RiskLevel::LOW:
            return "LOW";
        case RiskLevel::MEDIUM:
            return "MEDIUM";
        case RiskLevel::HIGH:
            return "HIGH";
        case RiskLevel::CRITICAL:
            return "CRITICAL";
        default:
            return "UNKNOWN";
    }
}

std::string AssetScoring::generateRiskSummary(
    double missingRisk,
    double inactivityRisk,
    double abnormalRisk,
    int repairCount) {

    std::vector<std::string> risks;

    if (missingRisk > 0.7) {
        risks.push_back("资产有丢失风险");
    } else if (missingRisk > 0.4) {
        risks.push_back("需要关注资产位置");
    }

    if (inactivityRisk > 0.6) {
        risks.push_back("资产长期闲置");
    }

    if (abnormalRisk > 0.5) {
        risks.push_back("存在异常行为");
    }

    if (repairCount > 3) {
        risks.push_back("维修记录较多");
    }

    if (risks.empty()) {
        return "资产状态良好";
    }

    std::ostringstream oss;
    for (size_t i = 0; i < risks.size(); ++i) {
        if (i > 0) oss << "；";
        oss << risks[i];
    }

    return oss.str();
}

} // namespace analytics