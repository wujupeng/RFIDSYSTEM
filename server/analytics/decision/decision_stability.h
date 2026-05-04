#pragma once
#include <string>
#include <map>
#include <vector>
#include <chrono>
#include <mutex>
#include "decision_engine.h"

namespace analytics {

enum class DecisionMode {
    SHADOW,       // 影子模式：只记录，不执行
    SUGGESTION,   // 建议模式：展示给用户，人工确认
    AUTO          // 自动模式：自动执行
};

struct DecisionRecord {
    int asset_id;
    std::string asset_name;
    ActionType action_type;
    std::string action;
    std::string reason;
    int priority;
    std::string suggested_location;
    std::chrono::system_clock::time_point timestamp;
    bool was_executed;
    bool was_ignored;
    std::string executed_by;
    DecisionMode mode;
};

struct CooldownConfig {
    int inspect_cooldown_minutes = 30;
    int alert_cooldown_minutes = 10;
    int idle_cooldown_minutes = 60;
    int maintenance_cooldown_minutes = 120;
    int notification_cooldown_minutes = 60;
};

class DecisionStabilityManager {
public:
    static DecisionStabilityManager& instance();

    void setMode(DecisionMode mode) { mode_ = mode; }
    DecisionMode getMode() const { return mode_; }

    void setCooldownConfig(const CooldownConfig& config) { cooldown_config_ = config; }
    CooldownConfig getCooldownConfig() const { return cooldown_config_; }

    bool isInCooldown(int assetId, ActionType actionType);

    Decision makeStableDecision(
        int assetId,
        const std::string& assetName,
        const std::string& currentLocation,
        const RiskScore& riskScore,
        const AssetScore& assetScore,
        bool inIllegalLocation = false,
        int moveCount24h = 0,
        int missingHours = 0,
        double dailyAvgScans = 0.0,
        const std::string& assetType = "normal",
        bool isBackup = false);

    void recordDecisionExecution(int assetId, bool executed, bool ignored = false, const std::string& user = "");

    DecisionRecord getLastDecision(int assetId) const;
    std::vector<DecisionRecord> getRecentDecisions(int assetId, int count = 5) const;

    void clearHistory() { decision_history_.clear(); }

    struct StabilityStats {
        int total_decisions = 0;
        int cooldown_hits = 0;
        int executions = 0;
        int ignored = 0;
        double adoption_rate = 0.0;
    };
    StabilityStats getStats() const;

private:
    DecisionStabilityManager()
        : mode_(DecisionMode::SHADOW)
        , last_cleanup_(std::chrono::system_clock::now()) {}

    DecisionStabilityManager(const DecisionStabilityManager&) = delete;
    DecisionStabilityManager& operator=(const DecisionStabilityManager&) = delete;

    int getCooldownMinutes(ActionType actionType);
    void cleanupOldRecords();

    bool isInCooldownInternal(int assetId, ActionType actionType,
                              std::chrono::system_clock::time_point now);
    DecisionRecord getLastDecisionInternal(int assetId) const;

    std::string generateConsistentReason(ActionType actionType, const RiskScore& riskScore,
                                        const AssetScore& assetScore, int missingHours);

    DecisionMode mode_;
    CooldownConfig cooldown_config_;

    std::map<int, std::vector<DecisionRecord>> decision_history_;
    std::chrono::system_clock::time_point last_cleanup_;

    mutable std::mutex mutex_;
};

} // namespace analytics