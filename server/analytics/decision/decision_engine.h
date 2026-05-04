#pragma once
#include <string>
#include <vector>
#include "../predictor/asset_predictor.h"
#include "../scoring/asset_score.h"

namespace analytics {

enum class ActionType {
    INSPECT,        // 立即巡检
    CHECK_USAGE,    // 检查使用情况
    SECURITY_ALERT, // 安全告警
    RELOCATE,       // 重新分配位置
    MAINTENANCE,    // 维护检查
    NOTIFICATION,   // 通知
    NO_ACTION       // 无需行动
};

struct Decision {
    int asset_id;
    std::string asset_name;
    ActionType action_type;
    std::string action;
    std::string reason;
    int priority;           // 1~5，1最高
    std::string suggested_location;
    bool is_shadow_mode;    // 影子模式：记录但不执行
};

struct DecisionResult {
    std::vector<Decision> high_priority;
    std::vector<Decision> medium_priority;
    std::vector<Decision> low_priority;
};

class DecisionEngine {
public:
    static DecisionEngine& instance();

    // 主要决策入口
    Decision makeDecision(
        int assetId,
        const std::string& assetName,
        const std::string& currentLocation,
        const RiskScore& riskScore,
        const AssetScore& assetScore,
        bool inIllegalLocation = false,
        int moveCount24h = 0,  // 去抖动后的移动次数
        int missingHours = 0,
        double dailyAvgScans = 0.0,  // 新增：日常扫描频率（判断活跃）
        const std::string& assetType = "normal",  // 新增：资产类型
        bool isBackup = false);  // 新增：是否备用设备

    DecisionResult analyzeAllAssets(const std::vector<Decision>& decisions);

    std::string getActionTypeString(ActionType type);

    // 配置阈值
    void setMissingRiskThreshold(double threshold) { missing_risk_threshold_ = threshold; }
    void setAbnormalRiskThreshold(double threshold) { abnormal_risk_threshold_ = threshold; }
    void setInactiveRiskThreshold(double threshold) { inactive_risk_threshold_ = threshold; }
    void setActiveAssetThreshold(double threshold) { active_asset_threshold_ = threshold; }

private:
    DecisionEngine() = default;
    DecisionEngine(const DecisionEngine&) = delete;
    DecisionEngine& operator=(const DecisionEngine&) = delete;

    // 🔴 修复1：只对活跃设备判断丢失
    Decision handleMissingRisk(
        int assetId, const std::string& assetName,
        const std::string& currentLocation,
        double missingRisk, int missingHours,
        double dailyAvgScans);

    // 🔴 修复2：备用设备不触发闲置
    Decision handleInactiveRisk(
        int assetId, const std::string& assetName,
        const std::string& currentLocation,
        double inactivityRisk, bool isBackup);

    // 🔴 修复3：使用去抖动后的移动次数
    Decision handleAbnormalRisk(
        int assetId, const std::string& assetName,
        const std::string& currentLocation,
        double abnormalRisk, int moveCount24h);

    Decision handleIllegalLocation(
        int assetId, const std::string& assetName,
        const std::string& currentLocation);

    Decision handleLowHealth(
        int assetId, const std::string& assetName,
        const std::string& currentLocation,
        double healthScore);

    // 配置参数
    double missing_risk_threshold_ = 0.8;
    double abnormal_risk_threshold_ = 0.7;
    double inactive_risk_threshold_ = 0.7;
    double active_asset_threshold_ = 1.0;  // 日均扫描 >1次才算活跃
};

} // namespace analytics