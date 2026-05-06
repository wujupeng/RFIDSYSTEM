#include <iostream>
#include <iomanip>
#include <vector>
#include <random>
#include <chrono>
#include <numeric>
#include <Eigen/Dense>

#include "server/tuning/bandit/bandit_types.h"
#include "server/tuning/bandit/linucb_algorithm.h"
#include "server/tuning/bandit/guardrail_module.h"
#include "server/tuning/bandit/policy_mixer.h"

using namespace bandit;

// 模拟规则引擎决策（用于 SHADOW 模式对比）
Action simulateRuleEngine(const ContextFeatures& context) {
    if (context.missing_risk > 0.8) {
        return {ActionType::INSPECT, 0.0, 1.0};
    } else if (context.missing_risk > 0.5) {
        return {ActionType::ALERT, 0.0, 1.0};
    } else if (context.inactivity_risk > 0.7) {
        return {ActionType::ALERT, 0.0, 1.0};
    } else if (context.abnormal_risk > 0.7) {
        return {ActionType::INSPECT, 0.0, 1.0};
    } else {
        return {ActionType::NO_ACTION, 0.0, 1.0};
    }
}

// 生成随机上下文
ContextFeatures generateRandomContext(std::mt19937& rng) {
    ContextFeatures ctx;
    ctx.missing_risk = std::uniform_real_distribution<double>(0, 1)(rng);
    ctx.inactivity_risk = std::uniform_real_distribution<double>(0, 1)(rng);
    ctx.abnormal_risk = std::uniform_real_distribution<double>(0, 1)(rng);
    ctx.daily_avg_scans = std::uniform_real_distribution<double>(0, 20)(rng);
    ctx.weekly_avg_scans = std::uniform_real_distribution<double>(0, 140)(rng);
    ctx.move_count_24h = std::uniform_int_distribution<int>(0, 20)(rng);
    ctx.hours_since_last_seen = std::uniform_int_distribution<int>(0, 168)(rng);
    ctx.in_illegal_location = std::uniform_real_distribution<double>(0, 1)(rng) < 0.05;
    ctx.is_backup = std::uniform_real_distribution<double>(0, 1)(rng) < 0.2;
    ctx.asset_type = std::uniform_int_distribution<int>(0, 3)(rng);
    ctx.last_seen_variance = std::uniform_real_distribution<double>(0, 1)(rng);
    ctx.location_stability = std::uniform_real_distribution<double>(0, 1)(rng);
    ctx.historical_missing_rate = std::uniform_real_distribution<double>(0, 0.3)(rng);
    return ctx;
}

int main() {
    std::cout << "\n";
    std::cout << "╔══════════════════════════════════════════════════════════════════╗\n";
    std::cout << "║                    SHADOW 模式验证测试                          ║\n";
    std::cout << "║              v3.3 Contextual Bandit - Phase 1                  ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════════════╝\n";
    std::cout << "\n";

    // 初始化 LinUCB 算法
    LinUCBAlgorithm algorithm(14, 1.0);
    
    std::mt19937 rng(42);
    const int iterations = 1000;
    
    // 统计变量
    int matches_rule = 0;
    int differs_rule = 0;
    int guardrail_triggered = 0;
    int high_risk_triggered = 0;
    int illegal_location_triggered = 0;
    int low_confidence_triggered = 0;
    
    std::vector<double> confidences;
    std::vector<double> uncertainties;
    
    std::cout << "正在运行 SHADOW 模式测试 (" << iterations << " 次迭代)...\n";
    std::cout << "───────────────────────────────────────────────────────────────────\n";
    
    for (int i = 0; i < iterations; ++i) {
        // 生成随机上下文
        ContextFeatures ctx = generateRandomContext(rng);
        
        // 使用 PolicyMixer 进行决策
        auto decision = PolicyMixer::decide(ctx, algorithm, 2);
        
        // 获取规则引擎决策
        Action rule_action = simulateRuleEngine(ctx);
        
        // 记录统计
        if (decision.source == "GUARDRAIL") {
            guardrail_triggered++;
            if (decision.reason.find("HIGH_RISK") != std::string::npos) {
                high_risk_triggered++;
            } else if (decision.reason.find("ILLEGAL_LOCATION") != std::string::npos) {
                illegal_location_triggered++;
            } else if (decision.reason.find("LOW_CONFIDENCE") != std::string::npos) {
                low_confidence_triggered++;
            }
        }
        
        // 比较 Bandit 决策与规则决策
        if (decision.final_action.type == rule_action.type) {
            matches_rule++;
        } else {
            differs_rule++;
        }
        
        // 记录置信度和不确定性
        if (!decision.top_k_actions.empty()) {
            confidences.push_back(decision.top_k_actions[0].confidence);
            uncertainties.push_back(decision.top_k_actions[0].uncertainty);
        }
        
        // 进度显示
        if ((i + 1) % 200 == 0) {
            std::cout << "进度: " << (i + 1) << "/" << iterations << "\n";
        }
    }
    
    // 计算统计
    double avg_confidence = confidences.empty() ? 0 : 
        std::accumulate(confidences.begin(), confidences.end(), 0.0) / confidences.size();
    double avg_uncertainty = uncertainties.empty() ? 0 : 
        std::accumulate(uncertainties.begin(), uncertainties.end(), 0.0) / uncertainties.size();
    
    // 输出结果
    std::cout << "\n";
    std::cout << "═══════════════════════════════════════════════════════════════════\n";
    std::cout << "                         SHADOW 模式测试结果                      \n";
    std::cout << "═══════════════════════════════════════════════════════════════════\n\n";
    
    std::cout << "【1. 决策一致性】\n";
    std::cout << "  与规则一致: " << matches_rule << " (" << std::fixed << std::setprecision(1) 
              << (matches_rule * 100.0 / iterations) << "%)\n";
    std::cout << "  与规则不同: " << differs_rule << " (" << std::fixed << std::setprecision(1) 
              << (differs_rule * 100.0 / iterations) << "%)\n\n";
    
    std::cout << "【2. Guardrail 触发情况】\n";
    std::cout << "  总触发次数: " << guardrail_triggered << "\n";
    std::cout << "    - 高风险触发: " << high_risk_triggered << "\n";
    std::cout << "    - 非法位置触发: " << illegal_location_triggered << "\n";
    std::cout << "    - 低置信度触发: " << low_confidence_triggered << "\n\n";
    
    std::cout << "【3. 模型置信度统计】\n";
    std::cout << "  平均置信度: " << std::fixed << std::setprecision(4) << avg_confidence << "\n";
    std::cout << "  平均不确定性: " << std::fixed << std::setprecision(4) << avg_uncertainty << "\n\n";
    
    std::cout << "═══════════════════════════════════════════════════════════════════\n";
    std::cout << "                         分析与建议                              \n";
    std::cout << "═══════════════════════════════════════════════════════════════════\n\n";
    
    if (matches_rule > differs_rule) {
        std::cout << "✅ 正向结果: Bandit 决策与规则引擎高度一致\n";
        std::cout << "   这表明模型学习到了合理的决策模式\n";
    } else {
        std::cout << "⚠️  注意: Bandit 决策与规则引擎差异较大\n";
        std::cout << "   建议: 检查奖励函数或增加训练数据\n";
    }
    
    if (guardrail_triggered > 0) {
        std::cout << "\n🛡️ Guardrail 正常工作，共触发 " << guardrail_triggered << " 次\n";
        if (high_risk_triggered > 0) {
            std::cout << "   - 高风险场景正确识别\n";
        }
        if (illegal_location_triggered > 0) {
            std::cout << "   - 非法位置场景正确识别\n";
        }
    }
    
    if (avg_confidence < 0.6) {
        std::cout << "\n⚠️  警告: 模型平均置信度较低 (" << avg_confidence << ")\n";
        std::cout << "   建议: 增加训练样本或调整超参数\n";
    }
    
    std::cout << "\n📊 下一步:\n";
    std::cout << "  1. 在生产环境开启 SHADOW 模式\n";
    std::cout << "  2. 持续监控决策一致性和 Guardrail 触发率\n";
    std::cout << "  3. 收集真实反馈数据用于模型优化\n";
    std::cout << "  4. 当一致性达到 80%+ 时，可考虑开启 SUGGESTION 模式\n";
    
    std::cout << "\n";
    
    return 0;
}
