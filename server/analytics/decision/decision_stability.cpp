#include "decision_stability.h"
#include "../../core/logger.h"
#include "../../repository/decision_repository.h"
#include "../../repository/rule_registry.h"
#include <algorithm>
#include <mutex>

namespace analytics {

DecisionStabilityManager& DecisionStabilityManager::instance() {
    static DecisionStabilityManager instance;
    return instance;
}

int DecisionStabilityManager::getCooldownMinutes(ActionType actionType) {
    switch (actionType) {
        case ActionType::SECURITY_ALERT:
            return cooldown_config_.alert_cooldown_minutes;
        case ActionType::INSPECT:
            return cooldown_config_.inspect_cooldown_minutes;
        case ActionType::CHECK_USAGE:
        case ActionType::RELOCATE:
            return cooldown_config_.idle_cooldown_minutes;
        case ActionType::MAINTENANCE:
            return cooldown_config_.maintenance_cooldown_minutes;
        case ActionType::NOTIFICATION:
            return cooldown_config_.notification_cooldown_minutes;
        default:
            return 60;
    }
}

bool DecisionStabilityManager::isInCooldown(int assetId, ActionType actionType) {
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = decision_history_.find(assetId);
    if (it == decision_history_.end() || it->second.empty()) {
        return false;
    }

    const auto& records = it->second;
    const auto& last_record = records.back();

    if (last_record.action_type != actionType) {
        return false;
    }

    auto now = std::chrono::system_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::minutes>(
        now - last_record.timestamp).count();

    int cooldown_minutes = getCooldownMinutes(actionType);

    if (elapsed < cooldown_minutes) {
        return true;
    }

    return false;
}

bool DecisionStabilityManager::isInCooldownInternal(int assetId, ActionType actionType,
                                                    std::chrono::system_clock::time_point now) {
    auto it = decision_history_.find(assetId);
    if (it == decision_history_.end() || it->second.empty()) {
        return false;
    }

    const auto& last_record = it->second.back();
    if (last_record.action_type != actionType) {
        return false;
    }

    auto elapsed = std::chrono::duration_cast<std::chrono::minutes>(
        now - last_record.timestamp).count();
    return elapsed < getCooldownMinutes(actionType);
}

Decision DecisionStabilityManager::makeStableDecision(
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

    std::lock_guard<std::mutex> lock(mutex_);

    cleanupOldRecords();

    Decision fresh_decision = DecisionEngine::instance().makeDecision(
        assetId, assetName, currentLocation,
        riskScore, assetScore,
        inIllegalLocation, moveCount24h, missingHours,
        dailyAvgScans, assetType, isBackup);

    auto now = std::chrono::system_clock::now();

    if (isInCooldownInternal(assetId, fresh_decision.action_type, now)) {
        DecisionRecord last = getLastDecisionInternal(assetId);
        if (last.asset_id != 0) {
            return last;
        }
    }

    DecisionRecord record;
    record.asset_id = assetId;
    record.asset_name = assetName;
    record.action_type = fresh_decision.action_type;
    record.action = fresh_decision.action;
    record.reason = fresh_decision.reason;
    record.priority = fresh_decision.priority;
    record.suggested_location = fresh_decision.suggested_location;
    record.timestamp = now;
    record.was_executed = false;
    record.was_ignored = false;
    record.mode = mode_;

    record.reason = generateConsistentReason(
        fresh_decision.action_type, riskScore, assetScore, missingHours);

    decision_history_[assetId].push_back(record);

    DecisionRecord db_record;
    db_record.asset_id = assetId;
    db_record.asset_name = assetName;
    db_record.location = currentLocation;
    db_record.action = fresh_decision.action;
    db_record.reason = record.reason;
    
    if (fresh_decision.priority <= 2) {
        db_record.risk_level = "HIGH";
    } else if (fresh_decision.priority <= 3) {
        db_record.risk_level = "MEDIUM";
    } else {
        db_record.risk_level = "LOW";
    }

    int decisionId = DecisionRepository::instance().insertWithId(db_record);

    DecisionSnapshot snapshot;
    snapshot.decision_id = decisionId;
    snapshot.asset_id = assetId;
    snapshot.risk_missing = riskScore.missing_risk;
    snapshot.risk_inactivity = riskScore.inactivity_risk;
    snapshot.risk_abnormal = riskScore.abnormal_risk;
    snapshot.score = fresh_decision.priority;
    snapshot.rule_version = "v2.3";
    snapshot.engine_version = "v2.3";
    
    snapshot.threshold_snapshot = {
        {"missing_threshold", 72},
        {"inactivity_threshold", 48},
        {"abnormal_threshold", 0.8},
        {"cooldown_alert", 60},
        {"cooldown_inspect", 30},
        {"cooldown_noaction", 15}
    };

    snapshot.rule_snapshot_json = RuleRegistry::instance().getCurrentRulesSnapshot();

    DecisionRepository::instance().insertSnapshot(snapshot);

    return fresh_decision;
}

DecisionRecord DecisionStabilityManager::getLastDecisionInternal(int assetId) const {
    auto it = decision_history_.find(assetId);
    if (it != decision_history_.end() && !it->second.empty()) {
        return it->second.back();
    }
    return DecisionRecord{};
}

void DecisionStabilityManager::cleanupOldRecords() {
    auto now = std::chrono::system_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::hours>(
        now - last_cleanup_).count();

    if (elapsed < 1) return;

    for (auto& pair : decision_history_) {
        auto& records = pair.second;
        auto cutoff = now - std::chrono::hours(24);
        records.erase(
            std::remove_if(records.begin(), records.end(),
                [&cutoff](const DecisionRecord& r) {
                    return r.timestamp < cutoff;
                }),
            records.end()
        );
    }

    last_cleanup_ = now;
}

std::string DecisionStabilityManager::generateConsistentReason(
    ActionType actionType,
    const RiskScore& riskScore,
    const AssetScore& assetScore,
    int missingHours) {

    switch (actionType) {
        case ActionType::INSPECT:
            if (riskScore.missing_risk > 0.8) {
                return "活跃设备超过" + std::to_string(missingHours) + "小时未扫描，丢失风险高";
            } else if (riskScore.missing_risk > 0.6) {
                return "活跃设备较长时间未扫描，建议确认位置";
            } else {
                return "设备扫描频率异常，建议检查状态";
            }

        case ActionType::SECURITY_ALERT:
            return "检测到安全相关异常，需要立即处理";

        case ActionType::CHECK_USAGE:
            return "设备移动频率超出正常范围，请检查使用情况";

        case ActionType::RELOCATE:
            return "设备长期闲置，建议重新分配或调整";

        case ActionType::MAINTENANCE:
            if (assetScore.health_score < 60) {
                return "设备健康度极低（" + std::to_string(static_cast<int>(assetScore.health_score)) + "），需要紧急维护";
            } else {
                return "设备健康度下降，建议进行计划维护";
            }

        case ActionType::NOTIFICATION:
            return "设备状态需要关注";

        default:
            return "设备状态正常";
    }
}

void DecisionStabilityManager::recordDecisionExecution(int assetId, bool executed, bool ignored, const std::string& user) {
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = decision_history_.find(assetId);
    if (it == decision_history_.end() || it->second.empty()) {
        return;
    }

    auto& last_record = it->second.back();
    last_record.was_executed = executed;
    last_record.was_ignored = ignored;
    last_record.executed_by = user;
}

DecisionRecord DecisionStabilityManager::getLastDecision(int assetId) const {
    std::lock_guard<std::mutex> lock(mutex_);
    return getLastDecisionInternal(assetId);
}

std::vector<DecisionRecord> DecisionStabilityManager::getRecentDecisions(int assetId, int count) const {
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = decision_history_.find(assetId);
    if (it == decision_history_.end()) {
        return {};
    }

    const auto& records = it->second;
    size_t start = records.size() > count ? records.size() - count : 0;

    std::vector<DecisionRecord> result;
    for (size_t i = start; i < records.size(); ++i) {
        result.push_back(records[i]);
    }
    return result;
}

DecisionStabilityManager::StabilityStats DecisionStabilityManager::getStats() const {
    std::lock_guard<std::mutex> lock(mutex_);

    StabilityStats stats;

    for (const auto& pair : decision_history_) {
        for (const auto& record : pair.second) {
            stats.total_decisions++;
            if (record.was_executed) stats.executions++;
            if (record.was_ignored) stats.ignored++;
        }
    }

    if (stats.total_decisions > 0) {
        stats.adoption_rate = (double)stats.executions / stats.total_decisions * 100.0;
    }

    return stats;
}

} // namespace analytics