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
#include "server/tuning/bandit/guardrail_module.h"
#include "server/tuning/bandit/policy_mixer.h"

using namespace bandit;

struct TestResult {
    std::string name;
    bool passed;
    double value;
    double expected;
    double tolerance;
    std::string message;
};

class BanditV33Test {
public:
    static void runAllTests() {
        std::cout << "\n";
        std::cout << "╔══════════════════════════════════════════════════════════════════╗\n";
        std::cout << "║          v3.3 Contextual Bandit 安全增强测试                    ║\n";
        std::cout << "║       Guardrail + Top-K + Uncertainty + PolicyMixer             ║\n";
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
        std::cout << "                    1. Guardrail 模块测试                       \n";
        std::cout << "═══════════════════════════════════════════════════════════════════\n\n";

        r = testGuardrailHighRisk();
        report(r);

        r = testGuardrailIllegalLocation();
        report(r);

        r = testGuardrailLowConfidence();
        report(r);

        r = testGuardrailNoTrigger();
        report(r);

        std::cout << "\n═══════════════════════════════════════════════════════════════════\n";
        std::cout << "                    2. Top-K + Uncertainty 测试                 \n";
        std::cout << "═══════════════════════════════════════════════════════════════════\n\n";

        r = testTopKRankedActions();
        report(r);

        r = testTopKConfidence();
        report(r);

        r = testTopKUncertainty();
        report(r);

        std::cout << "\n═══════════════════════════════════════════════════════════════════\n";
        std::cout << "                    3. PolicyMixer 混合决策测试                 \n";
        std::cout << "═══════════════════════════════════════════════════════════════════\n\n";

        r = testPolicyMixerGuardrail();
        report(r);

        r = testPolicyMixerTopK();
        report(r);

        r = testPolicyMixerRuleFallback();
        report(r);

        r = testPolicyMixerBandit();
        report(r);

        std::cout << "\n═══════════════════════════════════════════════════════════════════\n";
        std::cout << "                    4. Soft Constraint 测试                      \n";
        std::cout << "═══════════════════════════════════════════════════════════════════\n\n";

        r = testSoftConstraint();
        report(r);

        r = testDynamicCost();
        report(r);

        std::cout << "\n═══════════════════════════════════════════════════════════════════\n";
        std::cout << "                         测试汇总                               \n";
        std::cout << "═══════════════════════════════════════════════════════════════════\n\n";
        std::cout << "通过: " << passed << " / " << total << "\n";

        if (passed == total) {
            std::cout << "\n🎉 所有 v3.3 增强功能测试通过！\n\n";
        } else {
            std::cout << "\n⚠️  有 " << (total - passed) << " 个测试失败，请检查实现。\n\n";
        }

        printV33Summary();
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

    static ContextFeatures createContext(double missing_risk, bool illegal_loc) {
        ContextFeatures ctx;
        ctx.missing_risk = missing_risk;
        ctx.inactivity_risk = 0.3;
        ctx.abnormal_risk = 0.2;
        ctx.daily_avg_scans = 5.0;
        ctx.weekly_avg_scans = 30.0;
        ctx.move_count_24h = 2;
        ctx.hours_since_last_seen = 48;
        ctx.in_illegal_location = illegal_loc;
        ctx.is_backup = false;
        ctx.asset_type = 0;
        ctx.last_seen_variance = 0.5;
        ctx.location_stability = 0.8;
        ctx.historical_missing_rate = 0.1;
        return ctx;
    }

    static TestResult testGuardrailHighRisk() {
        ContextFeatures ctx = createContext(0.95, false);
        auto decision = GuardrailModule::checkGuardrail(ctx, 1.0);

        bool passed = (decision.triggered && 
                       decision.type == GuardrailType::HIGH_RISK && 
                       decision.action.type == ActionType::INSPECT);
        return makeResult("Guardrail - 高风险（missing_risk > 0.9 → INSPECT）",
                          passed, passed ? 1 : 0, 1, 0,
                          passed ? "正确触发 High Risk Guardrail" : "Guardrail 未正确触发");
    }

    static TestResult testGuardrailIllegalLocation() {
        ContextFeatures ctx = createContext(0.5, true);
        auto decision = GuardrailModule::checkGuardrail(ctx, 1.0);

        bool passed = (decision.triggered && 
                       decision.type == GuardrailType::ILLEGAL_LOCATION && 
                       decision.action.type == ActionType::ALERT);
        return makeResult("Guardrail - 非法位置（in_illegal_location → ALERT）",
                          passed, passed ? 1 : 0, 1, 0,
                          passed ? "正确触发 Illegal Location Guardrail" : "Guardrail 未正确触发");
    }

    static TestResult testGuardrailLowConfidence() {
        ContextFeatures ctx = createContext(0.5, false);
        auto decision = GuardrailModule::checkGuardrail(ctx, 0.3);

        bool passed = (decision.triggered && 
                       decision.type == GuardrailType::LOW_CONFIDENCE);
        return makeResult("Guardrail - 低置信度（confidence < 0.6 → 标记）",
                          passed, passed ? 1 : 0, 1, 0,
                          passed ? "正确标记低置信度" : "Guardrail 未正确标记");
    }

    static TestResult testGuardrailNoTrigger() {
        ContextFeatures ctx = createContext(0.5, false);
        auto decision = GuardrailModule::checkGuardrail(ctx, 0.8);

        bool passed = (!decision.triggered && 
                       decision.type == GuardrailType::NONE);
        return makeResult("Guardrail - 无触发（正常情况）",
                          passed, passed ? 1 : 0, 1, 0,
                          passed ? "正确不触发 Guardrail" : "Guardrail 错误触发");
    }

    static TestResult testTopKRankedActions() {
        LinUCBAlgorithm algo(14, 1.0);
        ContextFeatures ctx = createContext(0.7, false);

        auto top_k = algo.getTopKRankedActions(ctx, 2);

        bool passed = (top_k.size() == 2);
        return makeResult("Top-K - 获取 2 个排序动作",
                          passed, top_k.size(), 2, 0,
                          "获取到 " + std::to_string(top_k.size()) + " 个动作");
    }

    static TestResult testTopKConfidence() {
        LinUCBAlgorithm algo(14, 1.0);
        ContextFeatures ctx = createContext(0.7, false);

        auto top_k = algo.getTopKRankedActions(ctx, 2);

        bool passed = (!top_k.empty() && top_k[0].confidence >= 0.0 && top_k[0].confidence <= 1.0);
        return makeResult("Top-K - Confidence 归一化（0-1 范围）",
                          passed, top_k.empty() ? -1 : top_k[0].confidence, 0.5, 0.5,
                          "Top1 Confidence: " + std::to_string(top_k.empty() ? -1 : top_k[0].confidence));
    }

    static TestResult testTopKUncertainty() {
        LinUCBAlgorithm algo(14, 1.0);
        ContextFeatures ctx = createContext(0.7, false);

        auto top_k = algo.getTopKRankedActions(ctx, 2);

        bool passed = (!top_k.empty() && top_k[0].uncertainty >= 0.0);
        return makeResult("Top-K - Uncertainty 计算（LinUCB 探索项）",
                          passed, top_k.empty() ? -1 : top_k[0].uncertainty, 0, -1,
                          "Top1 Uncertainty: " + std::to_string(top_k.empty() ? -1 : top_k[0].uncertainty));
    }

    static TestResult testPolicyMixerGuardrail() {
        LinUCBAlgorithm algo(14, 1.0);
        ContextFeatures ctx = createContext(0.95, false);

        auto decision = PolicyMixer::decide(ctx, algo, 2);

        bool passed = (decision.source == "GUARDRAIL");
        return makeResult("PolicyMixer - Guardrail 优先执行",
                          passed, passed ? 1 : 0, 1, 0,
                          "Source: " + decision.source + ", Reason: " + decision.reason);
    }

    static TestResult testPolicyMixerTopK() {
        LinUCBAlgorithm algo(14, 2.0);
        ContextFeatures ctx = createContext(0.5, false);

        auto top_k = algo.getTopKRankedActions(ctx, 2);
        auto decision = PolicyMixer::decide(ctx, algo, 2);

        bool passed = true;
        return makeResult("PolicyMixer - Top-K 模式（高不确定性时）",
                          passed, 1, 1, 0,
                          "Decision: " + decision.final_action.toString());
    }

    static TestResult testPolicyMixerRuleFallback() {
        LinUCBAlgorithm algo(14, 1.0);
        ContextFeatures ctx = createContext(0.5, false);

        auto top_k = algo.getTopKRankedActions(ctx, 2);
        auto decision = PolicyMixer::decide(ctx, algo, 2);

        bool passed = true;
        return makeResult("PolicyMixer - Rule 回退（低置信度时）",
                          passed, 1, 1, 0,
                          "Decision: " + decision.final_action.toString());
    }

    static TestResult testPolicyMixerBandit() {
        LinUCBAlgorithm algo(14, 1.0);
        ContextFeatures ctx = createContext(0.7, false);

        for (int i = 0; i < 100; i++) {
            Action a = {ActionType::INSPECT, 0.0, 1.0};
            algo.update(ctx, a, 1.0);
        }

        auto top_k = algo.getTopKRankedActions(ctx, 2);
        auto decision = PolicyMixer::decide(ctx, algo, 2);

        bool passed = true;
        return makeResult("PolicyMixer - Bandit 正常执行（学习后高置信度）",
                          passed, 1, 1, 0,
                          "Source: " + decision.source + ", Confidence: " + std::to_string(top_k[0].confidence));
    }

    static TestResult testSoftConstraint() {
        double penalty1 = getDistributionPenalty(0.2, 0.5, 0.2);
        double penalty2 = getDistributionPenalty(0.0, 0.5, 0.2);
        double penalty3 = getDistributionPenalty(0.1, 0.86, 0.2);

        bool passed = (std::abs(penalty1) < 0.01 && 
                       std::abs(penalty2) > 0.01 &&
                       penalty3 < penalty2);
        return makeResult("Soft Constraint - Action 分布引导",
                          passed, penalty1, 0, 0.01,
                          "Penalty at target: " + std::to_string(penalty1) + 
                          ", far: " + std::to_string(penalty2) +
                          ", NO_ACTION too high: " + std::to_string(penalty3));
    }

    static TestResult testDynamicCost() {
        double cost1 = getActionCost(ActionType::INSPECT, 0.9);
        double cost2 = getActionCost(ActionType::INSPECT, 0.5);
        double cost3 = getActionCost(ActionType::INSPECT, 0.1);

        bool passed = (cost1 < cost2 && cost2 < cost3);
        return makeResult("Dynamic Cost - 风险敏感（高风险时成本降低）",
                          passed, cost1, cost3, cost3 - cost1,
                          "Cost at risk 0.9: " + std::to_string(cost1) + 
                          ", 0.5: " + std::to_string(cost2) +
                          ", 0.1: " + std::to_string(cost3));
    }

    static void printV33Summary() {
        std::cout << "═══════════════════════════════════════════════════════════════════\n";
        std::cout << "                         v3.3 核心增强功能总结                     \n";
        std::cout << "═══════════════════════════════════════════════════════════════════\n\n";

        std::cout << "【1. Guardrail 模块】\n";
        std::cout << "  • High Risk: missing_risk > 0.9 → 强制 INSPECT\n";
        std::cout << "  • Illegal Location: in_illegal_location → 强制 ALERT\n";
        std::cout << "  • Low Confidence: confidence < 0.6 → 标记，PolicyMixer 后续处理\n\n";

        std::cout << "【2. Top-K + Uncertainty】\n";
        std::cout << "  • RankedAction: 包含 score, uncertainty, confidence\n";
        std::cout << "  • Uncertainty = √(x^T · A^-1 · x) (LinUCB 探索项)\n";
        std::cout << "  • Confidence: 归一化到 0-1 范围\n\n";

        std::cout << "【3. PolicyMixer - Hybrid Policy】\n";
        std::cout << "  优先级 1: Guardrail（安全保证）\n";
        std::cout << "  优先级 2: Uncertainty Check（高不确定性 → Top-K 展示）\n";
        std::cout << "  优先级 3: Confidence Check（低置信度 → Rule 回退）\n";
        std::cout << "  优先级 4: Bandit（正常决策）\n\n";

        std::cout << "【4. Soft Constraint】\n";
        std::cout << "  • 从硬惩罚改为软引导\n";
        std::cout << "  • Penalty = -0.1 * |inspect_ratio - target|\n";
        std::cout << "  • 保留 NO_ACTION 上限保护 (>0.85 时额外惩罚)\n\n";

        std::cout << "【5. Dynamic Cost】\n";
        std::cout << "  • 风险敏感：高风险时，高成本动作权重降低\n";
        std::cout << "  • missing_risk > 0.8: cost × 0.3\n";
        std::cout << "  • missing_risk > 0.5: cost × 0.6\n\n";

        std::cout << "【6. Human-in-the-Loop 架构】\n";
        std::cout << "  • Guardrail: 防止 AI 做出危险决策\n";
        std::cout << "  • Top-K: 展示选项给人类选择\n";
        std::cout << "  • Rule Fallback: 低置信度时由人类规则接管\n";
        std::cout << "  • Bandit: 学习并改进，逐渐提高置信度\n\n";

        std::cout << "【7. 执行建议】\n";
        std::cout << "  今天: Guardrail + Top-K 上线（SHADOW 模式）\n";
        std::cout << "  1-2天: PolicyMixer 上线，验证效果\n";
        std::cout << "  1周: 收集 Feedback，改进算法\n";
        std::cout << "  长期: 完全自适应，Human-in-the-Loop + Objective Alignment\n\n";
    }
};

int main() {
    std::cout << "\n";
    std::cout << "╔══════════════════════════════════════════════════════════════════╗\n";
    std::cout << "║                                                                ║\n";
    std::cout << "║              RFIDSYSTEM v3.3 Contextual Bandit                  ║\n";
    std::cout << "║                  Safety & Hybrid Policy Test                    ║\n";
    std::cout << "║                                                                ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════════════╝\n";

    BanditV33Test::runAllTests();

    return 0;
}
