// tools/decision_sandbox.cpp
// RFID资产决策系统离线验证工具 - v2.3
// 核心升级：可量化验证 + 参数调优

#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <unordered_map>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cmath>
#include "../server/core/logger.h"
#include "../server/analytics/predictor/asset_predictor.h"
#include "../server/analytics/scoring/asset_score.h"
#include "../server/analytics/decision/decision_engine.h"
#include "../server/analytics/decision/action_generator.h"

using namespace analytics;

struct GroundTruthTestCase {
    int asset_id;
    std::string asset_name;
    std::string asset_type;
    std::string current_location;
    double hours_since_last_seen;
    double daily_avg_scans;
    double weekly_avg_scans;
    int abnormal_events;
    int location_changes_24h;
    int repair_count;
    bool in_illegal_location;
    bool is_backup;

    // Ground Truth（真实结果对照）
    std::string expected_action;
    std::string expected_reason;
    int importance;  // 权重：1=普通，2=重要，3=关键
};

struct PerActionStats {
    int total = 0;
    int correct = 0;

    // For Precision/Recall
    int true_positive = 0;   // 预测X，实际X
    int false_positive = 0;   // 预测X，实际不是X
    int false_negative = 0;   // 预测不是X，实际X

    double accuracy() const {
        return total > 0 ? (double)correct / total * 100.0 : 0.0;
    }

    double precision() const {
        int predicted_x = true_positive + false_positive;
        return predicted_x > 0 ? (double)true_positive / predicted_x * 100.0 : 0.0;
    }

    double recall() const {
        int actual_x = true_positive + false_negative;
        return actual_x > 0 ? (double)true_positive / actual_x * 100.0 : 0.0;
    }

    double f1_score() const {
        double p = precision();
        double r = recall();
        return (p + r) > 0 ? 2.0 * p * r / (p + r) : 0.0;
    }
};

struct ValidationReport {
    int total_tests = 0;
    int total_correct = 0;
    double overall_accuracy = 0.0;

    int weighted_correct = 0;
    int total_weight = 0;
    double weighted_accuracy = 0.0;

    std::unordered_map<std::string, PerActionStats> per_action_stats;

    int critical_errors = 0;
    int important_errors = 0;
};

class DecisionSandbox {
public:
    void loadTestData() {
        std::cout << "Loading Ground Truth test data...\n\n";

        test_cases_ = {
            // 场景1: 活跃设备丢失 - 关键测试
            GroundTruthTestCase{
                1, "Laptop-001", "laptop", "A1-01",
                72.0, 8.5, 55.0, 0, 2, 0, false, false,
                "INSPECT", "活跃设备72小时未扫描",
                3
            },

            // 场景2: 备用设备长期不扫描 - 不应告警
            GroundTruthTestCase{
                2, "Backup-Server", "backup", "Server-Room",
                720.0, 0.05, 0.3, 0, 0, 0, false, true,
                "NO_ACTION", "备用设备，闲置正常",
                3
            },

            // 场景3: 正常活跃设备 - 无需行动
            GroundTruthTestCase{
                3, "Scanner-005", "scanner", "A2-03",
                2.0, 15.2, 100.0, 0, 5, 0, false, false,
                "NO_ACTION", "正常活跃设备",
                2
            },

            // 场景4: 非法区域 - 关键测试
            GroundTruthTestCase{
                4, "Tool-Box-01", "tool", "Restricted-Zone",
                5.0, 3.0, 20.0, 0, 3, 0, true, false,
                "SECURITY_ALERT", "进入非法区域",
                3
            },

            // 场景5: 异常移动（去抖动后）
            GroundTruthTestCase{
                5, "Pallet-012", "pallet", "A3-01",
                1.0, 5.0, 30.0, 0, 25, 0, false, false,
                "CHECK_USAGE", "24小时内移动25次",
                2
            },

            // 场景6: 长期闲置非备用设备
            GroundTruthTestCase{
                6, "Printer-003", "printer", "B1-05",
                48.0, 0.2, 1.0, 0, 0, 2, false, false,
                "RELOCATE", "非备用设备长期闲置",
                2
            },

            // 场景7: 高健康度设备
            GroundTruthTestCase{
                7, "Monitor-002", "monitor", "A1-02",
                3.0, 10.0, 65.0, 0, 1, 0, false, false,
                "NO_ACTION", "健康度良好",
                1
            },

            // 场景8: 低健康度需要维护
            GroundTruthTestCase{
                8, "Old-Scanner", "scanner", "B2-01",
                6.0, 2.0, 12.0, 0, 1, 8, false, false,
                "MAINTENANCE", "健康度低，维修次数多",
                3
            },

            // 场景9: 低频设备不应触发丢失（修复后的关键测试）
            GroundTruthTestCase{
                9, "Rare-Equipment", "special", "C1-01",
                100.0, 0.3, 2.0, 0, 1, 0, false, false,
                "NO_ACTION", "低频设备，不应判丢失",
                3
            },

            // 场景10: 高风险但非活跃
            GroundTruthTestCase{
                10, "Idle-Device", "industrial", "D2-03",
                80.0, 0.4, 2.5, 0, 0, 0, false, false,
                "NO_ACTION", "非活跃设备不应触发",
                3
            },

            // 场景11: 连续异常事件
            GroundTruthTestCase{
                11, "Problem-Asset", "vehicle", "E1-01",
                4.0, 6.0, 40.0, 5, 30, 0, false, false,
                "SECURITY_ALERT", "异常事件+高移动",
                3
            },

            // 场景12: 中等风险需要关注
            GroundTruthTestCase{
                12, "Attention-Asset", "tool", "F2-02",
                30.0, 4.0, 25.0, 0, 2, 1, false, false,
                "NOTIFICATION", "中等风险需要关注",
                2
            }
        };

        std::cout << "Loaded " << test_cases_.size() << " Ground Truth test cases\n\n";
    }

    ValidationReport runValidation() {
        ValidationReport report;

        std::cout << "========================================\n";
        std::cout << "    决策系统量化验证报告 - v2.3\n";
        std::cout << "========================================\n\n";

        for (const auto& test_case : test_cases_) {
            report.total_tests++;
            report.total_weight += test_case.importance;

            std::cout << "【#" << test_case.asset_id << "】 " << test_case.asset_name;
            if (test_case.is_backup) std::cout << " [BACKUP]";
            std::cout << "\n";
            std::cout << "  期望动作: " << test_case.expected_action << "\n";
            std::cout << "  期望原因: " << test_case.expected_reason << "\n";

            // Step 1: 预测
            auto prediction = AssetPredictor::instance().predictAssetStatus(
                test_case.asset_id,
                test_case.hours_since_last_seen,
                test_case.daily_avg_scans,
                test_case.weekly_avg_scans,
                test_case.abnormal_events,
                test_case.location_changes_24h
            );

            // Step 2: 评分
            auto score = AssetScoring::instance().calculateAssetScore(
                prediction.risk_score.missing_risk,
                prediction.risk_score.inactivity_risk,
                prediction.risk_score.abnormal_risk,
                test_case.daily_avg_scans,
                test_case.repair_count
            );

            // Step 3: 决策
            auto decision = DecisionEngine::instance().makeDecision(
                test_case.asset_id,
                test_case.asset_name,
                test_case.current_location,
                prediction.risk_score,
                score,
                test_case.in_illegal_location,
                test_case.location_changes_24h,
                static_cast<int>(test_case.hours_since_last_seen),
                test_case.daily_avg_scans,
                test_case.asset_type,
                test_case.is_backup
            );

            std::string actual_action = DecisionEngine::instance().getActionTypeString(decision.action_type);

            std::cout << "  实际动作: " << actual_action << "\n";
            std::cout << "  决策原因: " << decision.reason << "\n";

            // 判定是否命中
            bool correct = isActionMatch(actual_action, test_case.expected_action);
            bool critical_mismatch = isCriticalMismatch(actual_action, test_case.expected_action);

            if (correct) {
                report.total_correct++;
                report.weighted_correct += test_case.importance;
                std::cout << "  ✅ 命中\n";
            } else {
                std::cout << "  ❌ 误判";
                if (critical_mismatch) {
                    std::cout << " [严重!]";
                    report.critical_errors++;
                } else if (test_case.importance >= 2) {
                    report.important_errors++;
                }
                std::cout << "\n";
            }

            // 更新分类统计
            updateStats(report.per_action_stats, actual_action,
                       test_case.expected_action, correct);

            std::cout << "\n";
        }

        // 计算总体指标
        report.overall_accuracy = (double)report.total_correct / report.total_tests * 100.0;
        report.weighted_accuracy = (double)report.weighted_correct / report.total_weight * 100.0;

        return report;
    }

    void printReport(const ValidationReport& report) {
        std::cout << "========================================\n";
        std::cout << "         量化验证结果\n";
        std::cout << "========================================\n\n";

        std::cout << "【总体指标】\n";
        std::cout << "  总测试数: " << report.total_tests << "\n";
        std::cout << "  命中数: " << report.total_correct << "\n";
        std::cout << "  准确率: " << std::fixed << std::setprecision(1)
                  << report.overall_accuracy << "%\n";
        std::cout << "  加权准确率: " << std::fixed << std::setprecision(1)
                  << report.weighted_accuracy << "%\n\n";

        std::cout << "【错误分析】\n";
        std::cout << "  严重错误: " << report.critical_errors << "\n";
        std::cout << "  重要错误: " << report.important_errors << "\n\n";

        std::cout << "【分类准确率 & Precision/Recall】\n";
        std::cout << std::setw(20) << std::left << "Action"
                  << std::setw(8) << "Total"
                  << std::setw(8) << "Acc%"
                  << std::setw(10) << "Precision%"
                  << std::setw(10) << "Recall%"
                  << std::setw(10) << "F1%" << "\n";
        std::cout << std::string(66, '-') << "\n";

        for (const auto& entry : report.per_action_stats) {
            const auto& stats = entry.second;
            if (stats.total > 0) {
                std::cout << std::setw(20) << std::left << entry.first
                          << std::setw(8) << stats.total
                          << std::setw(8) << std::fixed << std::setprecision(1) << stats.accuracy()
                          << std::setw(10) << stats.precision()
                          << std::setw(10) << stats.recall()
                          << std::setw(10) << stats.f1_score() << "\n";
            }
        }

        std::cout << "\n";

        // 判断是否满足上线标准
        std::cout << "【上线标准检查】\n";
        bool can_deploy = true;

        if (report.overall_accuracy >= 80.0) {
            std::cout << "  ✅ 总体准确率 " << report.overall_accuracy << "% >= 80%\n";
        } else {
            std::cout << "  ❌ 总体准确率 " << report.overall_accuracy << "% < 80%\n";
            can_deploy = false;
        }

        auto it = report.per_action_stats.find("INSPECT");
        if (it != report.per_action_stats.end() && it->second.total > 0) {
            if (it->second.accuracy() >= 75.0) {
                std::cout << "  ✅ INSPECT准确率 " << it->second.accuracy() << "% >= 75%\n";
            } else {
                std::cout << "  ❌ INSPECT准确率 " << it->second.accuracy() << "% < 75%\n";
                can_deploy = false;
            }
        }

        it = report.per_action_stats.find("SECURITY_ALERT");
        if (it != report.per_action_stats.end() && it->second.total > 0) {
            if (it->second.precision() >= 85.0) {
                std::cout << "  ✅ SECURITY_ALERT Precision " << it->second.precision()
                          << "% >= 85% (不乱报)\n";
            } else {
                std::cout << "  ⚠️  SECURITY_ALERT Precision " << it->second.precision()
                          << "% < 85% (可能乱报)\n";
            }
        }

        if (can_deploy && report.critical_errors == 0) {
            std::cout << "\n🎉 系统满足上线标准！\n";
        } else {
            std::cout << "\n⚠️  系统未满足上线标准，需要优化\n";
        }

        std::cout << "\n";
    }

    void runThresholdScan() {
        std::cout << "========================================\n";
        std::cout << "         阈值扫描优化\n";
        std::cout << "========================================\n\n";

        std::cout << std::setw(20) << std::left << "Threshold"
                  << std::setw(10) << "Accuracy%"
                  << std::setw(10) << "Weighted%"
                  << std::setw(10) << "Errors" << "\n";
        std::cout << std::string(50, '-') << "\n";

        double best_threshold = 0.8;
        double best_accuracy = 0.0;
        ValidationReport best_report;

        for (double t = 0.70; t <= 0.95; t += 0.05) {
            DecisionEngine::instance().setMissingRiskThreshold(t);

            auto report = runValidationForCurrentThreshold();

            std::cout << std::fixed << std::setprecision(2)
                      << std::setw(20) << std::left << t
                      << std::setw(10) << std::setprecision(1) << report.overall_accuracy
                      << std::setw(10) << report.weighted_accuracy
                      << std::setw(10) << (report.critical_errors + report.important_errors) << "\n";

            if (report.overall_accuracy > best_accuracy ||
                (report.overall_accuracy == best_accuracy &&
                 (report.critical_errors + report.important_errors) <
                 (best_report.critical_errors + best_report.important_errors))) {
                best_accuracy = report.overall_accuracy;
                best_threshold = t;
                best_report = report;
            }
        }

        std::cout << "\n【最优阈值】: " << std::fixed << std::setprecision(2) << best_threshold << "\n";
        std::cout << "【最优准确率】: " << std::fixed << std::setprecision(1) << best_accuracy << "%\n";
        std::cout << "【严重错误】: " << best_report.critical_errors << "\n";
        std::cout << "【重要错误】: " << best_report.important_errors << "\n\n";

        // 用最优阈值重新运行
        DecisionEngine::instance().setMissingRiskThreshold(best_threshold);
    }

private:
    std::vector<GroundTruthTestCase> test_cases_;

    bool isActionMatch(const std::string& actual, const std::string& expected) {
        if (actual == expected) return true;

        // 宽松匹配
        if (expected == "NO_ACTION" &&
            (actual == "NO_ACTION" || actual.find("无需行动") != std::string::npos)) {
            return true;
        }

        if (expected == "NOTIFICATION" &&
            (actual == "NOTIFICATION" || actual == "INSPECT")) {
            return true;
        }

        return false;
    }

    bool isCriticalMismatch(const std::string& actual, const std::string& expected) {
        // 关键误判：安全相关
        if (expected == "SECURITY_ALERT" && actual != "SECURITY_ALERT") return true;
        if (actual == "SECURITY_ALERT" && expected != "SECURITY_ALERT") return true;

        // 关键误判：该巡检但没巡检
        if (expected == "INSPECT" && actual == "NO_ACTION") return true;

        // 关键误判：正常设备误判为需要行动
        if (expected == "NO_ACTION" &&
            (actual == "INSPECT" || actual == "SECURITY_ALERT" || actual == "MAINTENANCE")) {
            return true;
        }

        return false;
    }

    void updateStats(std::unordered_map<std::string, PerActionStats>& stats,
                    const std::string& actual, const std::string& expected, bool correct) {
        // 更新actual的统计
        auto& actual_stats = stats[actual];
        actual_stats.total++;
        if (correct) {
            actual_stats.correct++;
            actual_stats.true_positive++;
        } else {
            actual_stats.false_positive++;
        }

        // 更新expected的统计（用于计算recall）
        if (expected != actual) {
            auto& expected_stats = stats[expected];
            expected_stats.total++;
            expected_stats.false_negative++;
        }
    }

    ValidationReport runValidationForCurrentThreshold() {
        ValidationReport report;

        for (const auto& test_case : test_cases_) {
            report.total_tests++;
            report.total_weight += test_case.importance;

            auto prediction = AssetPredictor::instance().predictAssetStatus(
                test_case.asset_id,
                test_case.hours_since_last_seen,
                test_case.daily_avg_scans,
                test_case.weekly_avg_scans,
                test_case.abnormal_events,
                test_case.location_changes_24h
            );

            auto score = AssetScoring::instance().calculateAssetScore(
                prediction.risk_score.missing_risk,
                prediction.risk_score.inactivity_risk,
                prediction.risk_score.abnormal_risk,
                test_case.daily_avg_scans,
                test_case.repair_count
            );

            auto decision = DecisionEngine::instance().makeDecision(
                test_case.asset_id,
                test_case.asset_name,
                test_case.current_location,
                prediction.risk_score,
                score,
                test_case.in_illegal_location,
                test_case.location_changes_24h,
                static_cast<int>(test_case.hours_since_last_seen),
                test_case.daily_avg_scans,
                test_case.asset_type,
                test_case.is_backup
            );

            std::string actual_action = DecisionEngine::instance().getActionTypeString(decision.action_type);
            bool correct = isActionMatch(actual_action, test_case.expected_action);

            if (correct) {
                report.total_correct++;
                report.weighted_correct += test_case.importance;
            }

            if (isCriticalMismatch(actual_action, test_case.expected_action)) {
                report.critical_errors++;
            } else if (test_case.importance >= 2 && !correct) {
                report.important_errors++;
            }

            updateStats(report.per_action_stats, actual_action,
                       test_case.expected_action, correct);
        }

        report.overall_accuracy = (double)report.total_correct / report.total_tests * 100.0;
        report.weighted_accuracy = (double)report.weighted_correct / report.total_weight * 100.0;

        return report;
    }
};

int main() {
    std::cout << "╔════════════════════════════════════════╗\n";
    std::cout << "║   RFID Decision System Validation Tool  ║\n";
    std::cout << "║            Version 2.3                 ║\n";
    std::cout << "╚════════════════════════════════════════╝\n\n";

    DecisionSandbox sandbox;

    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
    std::cout << "Step 1: 加载测试数据\n";
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
    sandbox.loadTestData();

    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
    std::cout << "Step 2: 运行量化验证\n";
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
    auto report = sandbox.runValidation();
    sandbox.printReport(report);

    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
    std::cout << "Step 3: 阈值扫描优化\n";
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
    sandbox.runThresholdScan();

    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
    std::cout << "Step 4: 使用最优阈值重新验证\n";
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
    auto final_report = sandbox.runValidation();
    sandbox.printReport(final_report);

    std::cout << "========================================\n";
    std::cout << "验证完成！\n";
    std::cout << "========================================\n";

    return 0;
}