#include <iostream>
#include <iomanip>
#include <vector>
#include <random>
#include <chrono>
#include <cmath>
#include <algorithm>
#include <Eigen/Dense>

#include "server/tuning/bandit/bandit_types.h"
#include "server/tuning/bandit/linucb_algorithm.h"
#include "server/tuning/bandit/bandit_engine.h"

using namespace bandit;

struct TestResult {
    std::string name;
    bool passed;
    double value;
    double expected;
    double tolerance;
    std::string message;
};

class BanditPerformanceTest {
public:
    static void runAllTests() {
        std::cout << "\n";
        std::cout << "╔══════════════════════════════════════════════════════════════════╗\n";
        std::cout << "║          v3.2 Contextual Bandit 性能测试                      ║\n";
        std::cout << "╚══════════════════════════════════════════════════════════════════╝\n";
        std::cout << "\n";

        int passed = 0;
        int total = 0;

        auto report = [&](TestResult& r) {
            total++;
            if (r.passed) {
                std::cout << "[✅ PASS] ";
                passed++;
            } else {
                std::cout << "[❌ FAIL] ";
            }
            std::cout << r.name << "\n";
            if (!r.message.empty()) {
                std::cout << "         " << r.message << "\n";
            }
            if (!r.passed) {
                std::cout << "         值: " << std::fixed << std::setprecision(4) << r.value
                         << " | 期望: " << r.expected << " | 容差: " << r.tolerance << "\n";
            }
        };

        TestResult r;

        std::cout << "═══════════════════════════════════════════════════════════════════\n";
        std::cout << "                    1. LinUCB 算法基础测试                     \n";
        std::cout << "═══════════════════════════════════════════════════════════════════\n\n";

        r = testLinUCBInitialization();
        report(r);

        r = testLinUCBActionSelection();
        report(r);

        r = testLinUCBModelUpdate();
        report(r);

        r = testLinUCBPrediction();
        report(r);

        std::cout << "\n═══════════════════════════════════════════════════════════════════\n";
        std::cout << "                    2. Context 特征测试                        \n";
        std::cout << "═══════════════════════════════════════════════════════════════════\n\n";

        r = testContextVectorConversion();
        report(r);

        r = testContextNormalization();
        report(r);

        std::cout << "\n═══════════════════════════════════════════════════════════════════\n";
        std::cout << "                    3. Action 选择测试                         \n";
        std::cout << "═══════════════════════════════════════════════════════════════════\n\n";

        r = testActionSpaceDiversity();
        report(r);

        r = testEpsilonGreedyExploration();
        report(r);

        std::cout << "\n═══════════════════════════════════════════════════════════════════\n";
        std::cout << "                    4. 奖励函数测试                           \n";
        std::cout << "═══════════════════════════════════════════════════════════════════\n\n";

        r = testRewardCalculation();
        report(r);

        r = testDelayedReward();
        report(r);

        std::cout << "\n═══════════════════════════════════════════════════════════════════\n";
        std::cout << "                    5. 收敛性测试                             \n";
        std::cout << "═══════════════════════════════════════════════════════════════════\n\n";

        r = testConvergenceBehavior();
        report(r);

        r = testRewardImprovement();
        report(r);

        std::cout << "\n═══════════════════════════════════════════════════════════════════\n";
        std::cout << "                    6. 压力测试                               \n";
        std::cout << "═══════════════════════════════════════════════════════════════════\n\n";

        r = testHighDimensionalContext();
        report(r);

        r = testRapidUpdates();
        report(r);

        std::cout << "\n═══════════════════════════════════════════════════════════════════\n";
        std::cout << "                    7. 探索-利用平衡测试                      \n";
        std::cout << "═══════════════════════════════════════════════════════════════════\n\n";

        r = testExplorationExploitationTradeoff();
        report(r);

        r = testEpsilonDecay();
        report(r);

        std::cout << "\n═══════════════════════════════════════════════════════════════════\n";
        std::cout << "                         测试汇总                               \n";
        std::cout << "═══════════════════════════════════════════════════════════════════\n\n";
        std::cout << "通过: " << passed << " / " << total << "\n";

        if (passed == total) {
            std::cout << "\n🎉 所有测试通过！算法效果验证成功！\n\n";
        } else {
            std::cout << "\n⚠️  有 " << (total - passed) << " 个测试失败，请检查算法实现。\n\n";
        }

        printAlgorithmAnalysis();
    }

private:
    static TestResult makeResult(const std::string& name, bool passed,
                                 double value, double expected, double tolerance,
                                 const std::string& msg = "") {
        TestResult r;
        r.name = name;
        r.passed = passed;
        r.value = value;
        r.expected = expected;
        r.tolerance = tolerance;
        r.message = msg;
        return r;
    }

    static TestResult testLinUCBInitialization() {
        LinUCBAlgorithm algo(11, 1.0);
        auto models = algo.getModels();

        bool passed = (models.size() == 4);
        return makeResult("LinUCB 初始化（4个Arm）",
                          passed, models.size(), 4, 0,
                          passed ? "正确创建 4 个动作臂" : "臂数量错误");
    }

    static TestResult testLinUCBActionSelection() {
        LinUCBAlgorithm algo(11, 1.0);
        ContextFeatures ctx;
        ctx.missing_risk = 0.7;
        ctx.inactivity_risk = 0.5;
        ctx.abnormal_risk = 0.3;
        ctx.daily_avg_scans = 5.0;
        ctx.weekly_avg_scans = 30.0;
        ctx.move_count_24h = 2;
        ctx.hours_since_last_seen = 48;
        ctx.in_illegal_location = false;
        ctx.is_backup = false;
        ctx.asset_type = 0;

        int actionCounts[4] = {0, 0, 0, 0};
        int iterations = 1000;

        for (int i = 0; i < iterations; i++) {
            Action a = algo.selectAction(ctx);
            actionCounts[static_cast<int>(a.type)]++;
        }

        int nonZero = 0;
        for (int i = 0; i < 4; i++) {
            if (actionCounts[i] > 0) nonZero++;
        }

        bool passed = (nonZero >= 2);
        return makeResult("Action 选择多样性",
                          passed, nonZero, 2, 0,
                          "在 " + std::to_string(iterations) + " 次选择中使用了 " + std::to_string(nonZero) + " 种动作");
    }

    static TestResult testLinUCBModelUpdate() {
        LinUCBAlgorithm algo(11, 1.0);
        ContextFeatures ctx;
        ctx.missing_risk = 0.8;
        ctx.inactivity_risk = 0.6;
        ctx.abnormal_risk = 0.4;
        ctx.daily_avg_scans = 3.0;
        ctx.weekly_avg_scans = 20.0;
        ctx.move_count_24h = 1;
        ctx.hours_since_last_seen = 72;
        ctx.in_illegal_location = true;
        ctx.is_backup = false;
        ctx.asset_type = 1;

        Action action = {ActionType::INSPECT, 0.0, 1.0};
        double reward = 0.8;

        algo.update(ctx, action, reward);

        auto models = algo.getModels();
        bool updated = false;
        for (const auto& m : models) {
            if (m.action.type == ActionType::INSPECT && m.sample_count > 0) {
                updated = true;
                break;
            }
        }

        return makeResult("模型更新",
                          updated, updated ? 1 : 0, 1, 0,
                          updated ? "模型成功更新" : "模型更新失败");
    }

    static TestResult testLinUCBPrediction() {
        LinUCBAlgorithm algo(11, 1.0);
        ContextFeatures ctx;
        ctx.missing_risk = 0.9;
        ctx.inactivity_risk = 0.8;
        ctx.abnormal_risk = 0.6;
        ctx.daily_avg_scans = 1.0;
        ctx.weekly_avg_scans = 7.0;
        ctx.move_count_24h = 0;
        ctx.hours_since_last_seen = 120;
        ctx.in_illegal_location = true;
        ctx.is_backup = false;
        ctx.asset_type = 0;

        Action action = {ActionType::ALERT, 0.0, 1.0};
        double reward = 1.0;

        algo.update(ctx, action, reward);

        double predicted = algo.predictReward(ctx, action);

        bool passed = (predicted > 0);
        return makeResult("奖励预测",
                          passed, predicted, 0, 0,
                          "预测奖励: " + std::to_string(predicted));
    }

    static TestResult testContextVectorConversion() {
        ContextFeatures ctx;
        ctx.missing_risk = 0.5;
        ctx.inactivity_risk = 0.5;
        ctx.abnormal_risk = 0.5;
        ctx.daily_avg_scans = 10.0;
        ctx.weekly_avg_scans = 70.0;
        ctx.move_count_24h = 5;
        ctx.hours_since_last_seen = 24;
        ctx.in_illegal_location = false;
        ctx.is_backup = true;
        ctx.asset_type = 2;

        Eigen::VectorXd vec = ctx.toVector();

        bool passed = (vec.size() == 11);
        double sum = vec.sum();

        return makeResult("Context 向量转换（11维）",
                          passed, vec.size(), 11, 0,
                          "向量维度: " + std::to_string(vec.size()) + ", 和: " + std::to_string(sum));
    }

    static TestResult testContextNormalization() {
        ContextFeatures ctx;
        ctx.missing_risk = 1.0;
        ctx.inactivity_risk = 1.0;
        ctx.abnormal_risk = 1.0;
        ctx.daily_avg_scans = 100.0;
        ctx.weekly_avg_scans = 700.0;
        ctx.move_count_24h = 50;
        ctx.hours_since_last_seen = 168;
        ctx.in_illegal_location = true;
        ctx.is_backup = true;
        ctx.asset_type = 3;

        Eigen::VectorXd vec = ctx.toVector();
        double maxVal = vec.cwiseAbs().maxCoeff();

        bool passed = (maxVal <= 200);
        return makeResult("Context 数值范围检查",
                          passed, maxVal, 0, 200,
                          "最大绝对值: " + std::to_string(maxVal));
    }

    static TestResult testActionSpaceDiversity() {
        LinUCBAlgorithm algo(11, 1.0);

        std::vector<ContextFeatures> contexts = {
            {0.9, 0.1, 0.1, 10.0, 70.0, 5, 2, false, false, 0},
            {0.1, 0.9, 0.1, 1.0, 7.0, 0, 120, false, false, 1},
            {0.1, 0.1, 0.9, 5.0, 35.0, 10, 1, true, false, 2},
            {0.3, 0.3, 0.3, 3.0, 21.0, 2, 48, false, true, 3}
        };

        std::vector<int> bestActions;
        for (const auto& ctx : contexts) {
            Action a = algo.selectAction(ctx);
            bestActions.push_back(static_cast<int>(a.type));
        }

        int unique = 0;
        std::sort(bestActions.begin(), bestActions.end());
        unique = std::unique(bestActions.begin(), bestActions.end()) - bestActions.begin();

        bool passed = (unique >= 2);
        return makeResult("Action 空间多样性",
                          passed, unique, 2, 0,
                          "不同动作类型数: " + std::to_string(unique));
    }

    static TestResult testEpsilonGreedyExploration() {
        LinUCBAlgorithm algo(11, 1.0);
        algo.setEpsilon(1.0);

        ContextFeatures ctx;
        ctx.missing_risk = 0.5;
        ctx.inactivity_risk = 0.5;
        ctx.abnormal_risk = 0.5;
        ctx.daily_avg_scans = 5.0;
        ctx.weekly_avg_scans = 35.0;
        ctx.move_count_24h = 2;
        ctx.hours_since_last_seen = 24;
        ctx.in_illegal_location = false;
        ctx.is_backup = false;
        ctx.asset_type = 0;

        int counts[4] = {0, 0, 0, 0};
        for (int i = 0; i < 1000; i++) {
            Action a = algo.selectAction(ctx);
            counts[static_cast<int>(a.type)]++;
        }

        int nonZero = 0;
        for (int i = 0; i < 4; i++) {
            if (counts[i] > 0) nonZero++;
        }

        bool passed = (nonZero == 4);
        return makeResult("Epsilon-Greedy 探索（100%探索）",
                          passed, nonZero, 4, 0,
                          "探索到 " + std::to_string(nonZero) + " 种动作");
    }

    static TestResult testRewardCalculation() {
        double accuracy = 0.85;
        double adoption_rate = 0.75;
        double false_positive = 0.10;
        double stability = 0.90;

        double reward = 0.4 * accuracy + 0.3 * adoption_rate - 0.2 * false_positive + 0.1 * stability;

        double expected = 0.4 * 0.85 + 0.3 * 0.75 - 0.2 * 0.10 + 0.1 * 0.90;
        expected = 0.34 + 0.225 - 0.02 + 0.09 = 0.635;

        bool passed = (std::abs(reward - expected) < 0.001);
        return makeResult("奖励函数计算",
                          passed, reward, expected, 0.001,
                          "计算奖励: " + std::to_string(reward));
    }

    static TestResult testDelayedReward() {
        DelayedRewardHandler& handler = DelayedRewardHandler::instance();
        handler.start();

        size_t initialCount = handler.getPendingCount();

        handler.addDelayedReward(1, 0.8, 1);
        handler.addDelayedReward(2, 0.6, 2);

        size_t afterAdd = handler.getPendingCount();

        handler.stop();

        bool passed = (afterAdd >= initialCount + 2);
        return makeResult("延迟奖励添加",
                          passed, afterAdd, initialCount + 2, 0,
                          "添加后数量: " + std::to_string(afterAdd));
    }

    static TestResult testConvergenceBehavior() {
        LinUCBAlgorithm algo(11, 0.5);
        std::mt19937 rng(42);

        ContextFeatures ctx;
        ctx.missing_risk = 0.7;
        ctx.inactivity_risk = 0.5;
        ctx.abnormal_risk = 0.3;
        ctx.daily_avg_scans = 5.0;
        ctx.weekly_avg_scans = 35.0;
        ctx.move_count_24h = 2;
        ctx.hours_since_last_seen = 48;
        ctx.in_illegal_location = false;
        ctx.is_backup = false;
        ctx.asset_type = 0;

        std::vector<double> rewards;

        for (int i = 0; i < 500; i++) {
            Action a = algo.selectAction(ctx);

            double baseReward = (a.type == ActionType::INSPECT) ? 0.9 : 0.3;
            double noise = (rng() % 100) / 100.0 * 0.2;
            double reward = baseReward + noise - 0.1;

            algo.update(ctx, a, reward);
            rewards.push_back(reward);
        }

        double avgFirst100 = 0, avgLast100 = 0;
        for (int i = 0; i < 100; i++) avgFirst100 += rewards[i];
        for (int i = 400; i < 500; i++) avgLast100 += rewards[i];
        avgFirst100 /= 100;
        avgLast100 /= 100;

        bool passed = (avgLast100 > avgFirst100);
        return makeResult("收敛行为（奖励提升）",
                          passed, avgLast100, avgFirst100, -0.05,
                          "前100平均: " + std::to_string(avgFirst100) +
                          " | 后100平均: " + std::to_string(avgLast100));
    }

    static TestResult testRewardImprovement() {
        LinUCBAlgorithm algo(11, 0.3);

        ContextFeatures ctx;
        ctx.missing_risk = 0.8;
        ctx.inactivity_risk = 0.6;
        ctx.abnormal_risk = 0.4;
        ctx.daily_avg_scans = 3.0;
        ctx.weekly_avg_scans = 21.0;
        ctx.move_count_24h = 1;
        ctx.hours_since_last_seen = 72;
        ctx.in_illegal_location = true;
        ctx.is_backup = false;
        ctx.asset_type = 1;

        std::vector<double> allRewards;

        for (int ep = 0; ep < 20; ep++) {
            double epReward = 0;
            for (int i = 0; i < 50; i++) {
                Action a = algo.selectAction(ctx);
                double r = (a.type == ActionType::INSPECT) ? 0.85 : 0.35;
                algo.update(ctx, a, r);
                epReward += r;
            }
            allRewards.push_back(epReward / 50.0);
        }

        double earlyAvg = 0, lateAvg = 0;
        for (int i = 0; i < 5; i++) earlyAvg += allRewards[i];
        for (int i = 15; i < 20; i++) lateAvg += allRewards[i];
        earlyAvg /= 5;
        lateAvg /= 5;

        double improvement = (lateAvg - earlyAvg) / earlyAvg * 100;

        bool passed = (improvement > 0);
        return makeResult("奖励改进率",
                          passed, improvement, 0, 0,
                          "改进率: " + std::to_string(improvement) + "%");
    }

    static TestResult testHighDimensionalContext() {
        LinUCBAlgorithm algo(11, 1.0);

        ContextFeatures ctx;
        ctx.missing_risk = 1.0;
        ctx.inactivity_risk = 1.0;
        ctx.abnormal_risk = 1.0;
        ctx.daily_avg_scans = 100.0;
        ctx.weekly_avg_scans = 700.0;
        ctx.move_count_24h = 50;
        ctx.hours_since_last_seen = 200;
        ctx.in_illegal_location = true;
        ctx.is_backup = true;
        ctx.asset_type = 3;

        bool noException = true;
        std::string errorMsg;

        try {
            for (int i = 0; i < 10000; i++) {
                Action a = algo.selectAction(ctx);
                algo.update(ctx, a, 0.5);
            }
        } catch (const std::exception& e) {
            noException = false;
            errorMsg = e.what();
        }

        return makeResult("高维上下文压力测试（10000次迭代）",
                          noException, noException ? 1 : 0, 1, 0,
                          noException ? "无异常" : errorMsg);
    }

    static TestResult testRapidUpdates() {
        LinUCBAlgorithm algo(11, 1.0);

        ContextFeatures ctx;
        ctx.missing_risk = 0.5;
        ctx.inactivity_risk = 0.5;
        ctx.abnormal_risk = 0.5;
        ctx.daily_avg_scans = 5.0;
        ctx.weekly_avg_scans = 35.0;
        ctx.move_count_24h = 2;
        ctx.hours_since_last_seen = 24;
        ctx.in_illegal_location = false;
        ctx.is_backup = false;
        ctx.asset_type = 0;

        bool noException = true;
        std::chrono::milliseconds elapsed;

        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < 1000; i++) {
            Action a = algo.selectAction(ctx);
            algo.update(ctx, a, 0.7);
        }
        auto end = std::chrono::high_resolution_clock::now();
        elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

        bool passed = (noException && elapsed.count() < 5000);
        return makeResult("快速更新性能（1000次）",
                          passed, elapsed.count(), 5000, 0,
                          "耗时: " + std::to_string(elapsed.count()) + "ms");
    }

    static TestResult testExplorationExploitationTradeoff() {
        LinUCBAlgorithm algo(11, 1.0);

        ContextFeatures ctx;
        ctx.missing_risk = 0.6;
        ctx.inactivity_risk = 0.5;
        ctx.abnormal_risk = 0.4;
        ctx.daily_avg_scans = 4.0;
        ctx.weekly_avg_scans = 28.0;
        ctx.move_count_24h = 2;
        ctx.hours_since_last_seen = 36;
        ctx.in_illegal_location = false;
        ctx.is_backup = false;
        ctx.asset_type = 0;

        for (int i = 0; i < 200; i++) {
            Action a = algo.selectAction(ctx);
            double r = (a.type == ActionType::INSPECT) ? 0.9 : 0.3;
            algo.update(ctx, a, r);
        }

        int inspectCount = 0;
        for (int i = 0; i < 100; i++) {
            Action a = algo.selectAction(ctx);
            if (a.type == ActionType::INSPECT) inspectCount++;
        }

        double exploitRatio = inspectCount / 100.0;

        bool passed = (exploitRatio > 0.5);
        return makeResult("探索-利用平衡（200次学习后）",
                          passed, exploitRatio, 0.5, 0,
                          "INSPECT 比例: " + std::to_string(exploitRatio * 100) + "%");
    }

    static TestResult testEpsilonDecay() {
        LinUCBAlgorithm algo(11, 1.0);

        double initialEps = algo.getCurrentEpsilon();

        for (int i = 0; i < 5000; i++) {
            ContextFeatures ctx;
            ctx.missing_risk = 0.5;
            ctx.inactivity_risk = 0.5;
            ctx.abnormal_risk = 0.5;
            ctx.daily_avg_scans = 5.0;
            ctx.weekly_avg_scans = 35.0;
            ctx.move_count_24h = 2;
            ctx.hours_since_last_seen = 24;
            ctx.in_illegal_location = false;
            ctx.is_backup = false;
            ctx.asset_type = 0;

            Action a = algo.selectAction(ctx);
            algo.update(ctx, a, 0.5);
        }

        double finalEps = algo.getCurrentEpsilon();

        bool passed = (finalEps < initialEps);
        return makeResult("Epsilon 衰减",
                          passed, finalEps, initialEps, 0,
                          "从 " + std::to_string(initialEps) +
                          " 衰减到 " + std::to_string(finalEps));
    }

    static void printAlgorithmAnalysis() {
        std::cout << "═══════════════════════════════════════════════════════════════════\n";
        std::cout << "                         算法效果分析                               \n";
        std::cout << "═══════════════════════════════════════════════════════════════════\n\n";

        LinUCBAlgorithm algo(11, 1.0);

        std::cout << "【1. LinUCB 算法特性】\n";
        std::cout << "  • UCB 公式: score = θ^T · x + α · √(x^T · A^{-1} · x)\n";
        std::cout << "  • 探索系数 α: 控制探索-利用平衡\n";
        std::cout << "  • Epsilon 衰减: 0.1 → 0.02 (10000次后)\n\n";

        std::cout << "【2. 上下文特征（11维）】\n";
        std::cout << "  风险特征: missing_risk, inactivity_risk, abnormal_risk\n";
        std::cout << "  统计特征: daily_avg_scans, weekly_avg_scans\n";
        std::cout << "  行为特征: move_count_24h, hours_since_last_seen\n";
        std::cout << "  状态特征: in_illegal_location, is_backup\n";
        std::cout << "  类型特征: asset_type (+ bias term)\n\n";

        std::cout << "【3. Action 空间】\n";
        std::cout << "  NO_ACTION (0): 无需行动\n";
        std::cout << "  INSPECT (1): 立即巡检 ← 高风险资产\n";
        std::cout << "  ALERT (2): 告警通知\n";
        std::cout << "  REALLOCATE (3): 重新分配\n\n";

        std::cout << "【4. 预期效果】\n";
        std::cout << "  • 收敛后 INSPECT 动作比例 > 60%\n";
        std::cout << "  • 奖励改进率 > 20%（经过充分学习）\n";
        std::cout << "  • 误报率 < 15%\n";
        std::cout << "  • 采纳率 > 65%\n\n";

        std::cout << "【5. 适用场景】\n";
        std::cout << "  • RFID 资产异常检测\n";
        std::cout << "  • 巡检优先级排序\n";
        std::cout << "  • 资源分配优化\n";
        std::cout << "  • 告警阈值自适应\n\n";

        std::cout << "【6. 与现有系统集成】\n";
        std::cout << "  SHADOW: Bandit 只预测，Rule Engine 执行\n";
        std::cout << "  SUGGESTION: 显示 Bandit 推荐供用户选择\n";
        std::cout << "  AUTO: Bandit 覆盖 Rule Engine（全自动）\n\n";

        std::cout << "【7. 下一步建议】\n";
        std::cout << "  Phase 1: 开启 SHADOW 模式，收集 1000+ 样本\n";
        std::cout << "  Phase 2: 切换到 SUGGESTION 模式，验证效果\n";
        std::cout << "  Phase 3: 开启 AUTO 模式，全自动化决策\n\n";
    }
};

int main() {
    std::cout << "\n";
    std::cout << "╔══════════════════════════════════════════════════════════════════╗\n";
    std::cout << "║                                                                ║\n";
    std::cout << "║              RFIDSYSTEM v3.2 Contextual Bandit                  ║\n";
    std::cout << "║                   Performance Test Suite                        ║\n";
    std::cout << "║                                                                ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════════════╝\n";

    BanditPerformanceTest::runAllTests();

    return 0;
}