#include "decision_engine.h"
#include "../../core/logger.h"
#include <algorithm>
#include <sstream>

namespace analytics {

DecisionEngine& DecisionEngine::instance() {
    static DecisionEngine instance;
    return instance;
}

std::string DecisionEngine::getActionTypeString(ActionType type) {
    switch (type) {
        case ActionType::INSPECT:
            return "INSPECT";
        case ActionType::CHECK_USAGE:
            return "CHECK_USAGE";
        case ActionType::SECURITY_ALERT:
            return "SECURITY_ALERT";
        case ActionType::RELOCATE:
            return "RELOCATE";
        case ActionType::MAINTENANCE:
            return "MAINTENANCE";
        case ActionType::NOTIFICATION:
            return "NOTIFICATION";
        case ActionType::NO_ACTION:
            return "NO_ACTION";
        default:
            return "UNKNOWN";
    }
}

// 🔴 修复1：只对活跃设备判断丢失
Decision DecisionEngine::handleMissingRisk(
    int assetId,
    const std::string& assetName,
    const std::string& currentLocation,
    double missingRisk,
    int missingHours,
    double dailyAvgScans) {

    Decision decision;
    decision.asset_id = assetId;
    decision.asset_name = assetName;
    decision.suggested_location = currentLocation;
    decision.is_shadow_mode = false;

    // 关键：只对活跃设备判断丢失
    bool isActive = dailyAvgScans > active_asset_threshold_;

    if (!isActive) {
        decision.action_type = ActionType::NO_ACTION;
        decision.action = "无需行动（非活跃设备）";
        decision.reason = "设备为低频设备，不触发丢失告警";
        decision.priority = 5;
        return decision;
    }

    if (missingRisk > missing_risk_threshold_ || missingHours > 72) {
        decision.action_type = ActionType::INSPECT;
        decision.action = "立即巡检";
        decision.reason = "活跃设备超过" + std::to_string(missingHours) + "小时未扫描，有丢失风险";
        decision.priority = 1;
    } else if (missingRisk > 0.6 || missingHours > 48) {
        decision.action_type = ActionType::INSPECT;
        decision.action = "尽快检查";
        decision.reason = "活跃设备较长时间未扫描，建议确认位置";
        decision.priority = 2;
    } else if (missingRisk > 0.4 || missingHours > 24) {
        decision.action_type = ActionType::NOTIFICATION;
        decision.action = "关注状态";
        decision.reason = "活跃设备超过24小时未扫描，建议关注";
        decision.priority = 3;
    } else {
        decision.action_type = ActionType::NO_ACTION;
        decision.action = "无需行动";
        decision.reason = "资产状态正常";
        decision.priority = 5;
    }

    return decision;
}

Decision DecisionEngine::handleAbnormalRisk(
    int assetId,
    const std::string& assetName,
    const std::string& currentLocation,
    double abnormalRisk,
    int moveCount24h) {

    Decision decision;
    decision.asset_id = assetId;
    decision.asset_name = assetName;
    decision.suggested_location = currentLocation;
    decision.is_shadow_mode = false;

    if (abnormalRisk > abnormal_risk_threshold_ || moveCount24h > 20) {
        decision.action_type = ActionType::SECURITY_ALERT;
        decision.action = "安全检查";
        decision.reason = "资产在24小时内移动" + std::to_string(moveCount24h) + "次（去抖动后），存在异常";
        decision.priority = 1;
    } else if (abnormalRisk > 0.5 || moveCount24h > 10) {
        decision.action_type = ActionType::CHECK_USAGE;
        decision.action = "检查使用";
        decision.reason = "资产移动频率较高，建议检查使用情况";
        decision.priority = 3;
    } else {
        decision.action_type = ActionType::NO_ACTION;
        decision.action = "无需行动";
        decision.reason = "资产行为正常";
        decision.priority = 5;
    }

    return decision;
}

// 🔴 修复2：备用设备不触发闲置
Decision DecisionEngine::handleInactiveRisk(
    int assetId,
    const std::string& assetName,
    const std::string& currentLocation,
    double inactivityRisk,
    bool isBackup) {

    Decision decision;
    decision.asset_id = assetId;
    decision.asset_name = assetName;
    decision.suggested_location = currentLocation;
    decision.is_shadow_mode = false;

    if (isBackup) {
        decision.action_type = ActionType::NO_ACTION;
        decision.action = "无需行动（备用设备）";
        decision.reason = "资产为备用设备，闲置为正常状态";
        decision.priority = 5;
        return decision;
    }

    if (inactivityRisk > inactive_risk_threshold_) {
        decision.action_type = ActionType::RELOCATE;
        decision.action = "重新分配";
        decision.reason = "资产长期闲置，建议重新分配使用";
        decision.priority = 4;
    } else if (inactivityRisk > 0.6) {
        decision.action_type = ActionType::NOTIFICATION;
        decision.action = "评估使用";
        decision.reason = "资产使用频率较低，建议评估是否需要调整";
        decision.priority = 4;
    } else {
        decision.action_type = ActionType::NO_ACTION;
        decision.action = "无需行动";
        decision.reason = "资产使用正常";
        decision.priority = 5;
    }

    return decision;
}

Decision DecisionEngine::handleIllegalLocation(
    int assetId,
    const std::string& assetName,
    const std::string& currentLocation) {

    Decision decision;
    decision.asset_id = assetId;
    decision.asset_name = assetName;
    decision.suggested_location = "";
    decision.is_shadow_mode = false;

    decision.action_type = ActionType::SECURITY_ALERT;
    decision.action = "立即移出";
    decision.reason = "资产出现在非法区域：" + currentLocation;
    decision.priority = 1;

    return decision;
}

Decision DecisionEngine::handleLowHealth(
    int assetId,
    const std::string& assetName,
    const std::string& currentLocation,
    double healthScore) {

    Decision decision;
    decision.asset_id = assetId;
    decision.asset_name = assetName;
    decision.suggested_location = currentLocation;
    decision.is_shadow_mode = false;

    if (healthScore < 60) {
        decision.action_type = ActionType::MAINTENANCE;
        decision.action = "紧急维护";
        decision.reason = "资产健康度极低 (" + std::to_string(static_cast<int>(healthScore)) + ")，需要检查";
        decision.priority = 2;
    } else if (healthScore < 80) {
        decision.action_type = ActionType::MAINTENANCE;
        decision.action = "计划维护";
        decision.reason = "资产健康度较低 (" + std::to_string(static_cast<int>(healthScore)) + ")，建议维护";
        decision.priority = 3;
    } else {
        decision.action_type = ActionType::NO_ACTION;
        decision.action = "无需行动";
        decision.reason = "资产健康度良好";
        decision.priority = 5;
    }

    return decision;
}

Decision DecisionEngine::makeDecision(
    int assetId,
    const std::string& assetName,
    const std::string& currentLocation,
    const RiskScore& riskScore,
    const AssetScore& assetScore,
    bool inIllegalLocation,
    int moveCount24h,
    int missingHours,
    double dailyAvgScans,
    const std::string& assetType,
    bool isBackup) {

    if (inIllegalLocation) {
        return handleIllegalLocation(assetId, assetName, currentLocation);
    }

    std::vector<Decision> possibleDecisions;

    if (riskScore.missing_risk > 0.4 || missingHours > 24) {
        possibleDecisions.push_back(
            handleMissingRisk(assetId, assetName, currentLocation,
                             riskScore.missing_risk, missingHours, dailyAvgScans)
        );
    }

    if (riskScore.abnormal_risk > 0.5 || moveCount24h > 10) {
        possibleDecisions.push_back(
            handleAbnormalRisk(assetId, assetName, currentLocation,
                             riskScore.abnormal_risk, moveCount24h)
        );
    }

    if (riskScore.inactivity_risk > 0.6) {
        possibleDecisions.push_back(
            handleInactiveRisk(assetId, assetName, currentLocation, riskScore.inactivity_risk, isBackup)
        );
    }

    if (assetScore.health_score < 80) {
        possibleDecisions.push_back(
            handleLowHealth(assetId, assetName, currentLocation, assetScore.health_score)
        );
    }

    if (possibleDecisions.empty()) {
        Decision defaultDecision;
        defaultDecision.asset_id = assetId;
        defaultDecision.asset_name = assetName;
        defaultDecision.action_type = ActionType::NO_ACTION;
        defaultDecision.action = "无需行动";
        defaultDecision.reason = "资产状态正常";
        defaultDecision.priority = 5;
        defaultDecision.suggested_location = currentLocation;
        defaultDecision.is_shadow_mode = false;
        return defaultDecision;
    }

    std::sort(possibleDecisions.begin(), possibleDecisions.end(),
        [](const Decision& a, const Decision& b) {
            return a.priority < b.priority;
        });

    spdlog::info("Decision for asset {}: action={}, priority={}, reason={}",
        assetId, possibleDecisions[0].action, possibleDecisions[0].priority, possibleDecisions[0].reason);

    return possibleDecisions[0];
}

DecisionResult DecisionEngine::analyzeAllAssets(const std::vector<Decision>& decisions) {
    DecisionResult result;

    for (const auto& decision : decisions) {
        if (decision.priority <= 2) {
            result.high_priority.push_back(decision);
        } else if (decision.priority <= 3) {
            result.medium_priority.push_back(decision);
        } else {
            result.low_priority.push_back(decision);
        }
    }

    spdlog::info("Decision analysis: high={}, medium={}, low={}",
        result.high_priority.size(), result.medium_priority.size(), result.low_priority.size());

    return result;
}

} // namespace analytics