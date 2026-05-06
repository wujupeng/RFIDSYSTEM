#pragma once

#include <vector>
#include <string>
#include <Eigen/Dense>

namespace bandit {

// 动作类型枚举
enum class ActionType {
    NO_ACTION = 0,
    INSPECT = 1,
    ALERT = 2,
    REALLOCATE = 3
};

// 动作结构（包含参数调整）
struct Action {
    ActionType type;
    double threshold_adjustment;  // 阈值调整 [-0.5, 0.5]
    double confidence_boost;      // 置信度调节 [0.8, 1.2]
    
    std::string toString() const {
        switch (type) {
            case ActionType::NO_ACTION: return "NO_ACTION";
            case ActionType::INSPECT: return "INSPECT";
            case ActionType::ALERT: return "ALERT";
            case ActionType::REALLOCATE: return "REALLOCATE";
            default: return "UNKNOWN";
        }
    }
    
    static ActionType fromString(const std::string& str) {
        if (str == "NO_ACTION") return ActionType::NO_ACTION;
        if (str == "INSPECT") return ActionType::INSPECT;
        if (str == "ALERT") return ActionType::ALERT;
        if (str == "REALLOCATE") return ActionType::REALLOCATE;
        return ActionType::NO_ACTION;
    }
};

// 上下文特征向量
struct ContextFeatures {
    // 风险特征
    double missing_risk;        // 丢失风险 [0, 1]
    double inactivity_risk;     // 闲置风险 [0, 1]
    double abnormal_risk;       // 异常行为风险 [0, 1]
    
    // 扫描统计特征
    double daily_avg_scans;     // 日均扫描次数
    double weekly_avg_scans;    // 周均扫描次数
    
    // 行为特征
    int move_count_24h;         // 24小时移动次数
    int hours_since_last_seen;  // 距离上次扫描的小时数
    
    // 状态特征
    bool in_illegal_location;   // 是否在非法位置
    bool is_backup;             // 是否为备用设备
    
    // 类型特征
    int asset_type;             // 资产类型编码：0=电脑, 1=服务器, 2=生产设备, 3=其他

    // 新增：增强区分度的特征
    double last_seen_variance;     // 最近扫描间隔波动
    double location_stability;     // 位置稳定性
    double historical_missing_rate; // 历史丢失率

    // 转换为 Eigen 向量
    Eigen::VectorXd toVector() const {
        Eigen::VectorXd vec(14);
        vec << missing_risk, inactivity_risk, abnormal_risk,
               daily_avg_scans, weekly_avg_scans,
               static_cast<double>(move_count_24h), 
               static_cast<double>(hours_since_last_seen),
               in_illegal_location ? 1.0 : 0.0,
               is_backup ? 1.0 : 0.0,
               static_cast<double>(asset_type),
               last_seen_variance,
               location_stability,
               historical_missing_rate,
               1.0;  // bias term
        return vec;
    }
};

// v3.3: RankedAction - 带不确定性和置信度的排序动作
struct RankedAction {
    Action action;
    double score;
    double uncertainty;  // √(x^T A^-1 x) - LinUCB 不确定性
    double confidence;   // normalized(score) - 置信度
    
    std::string toString() const {
        return action.toString() + " (score:" + std::to_string(score) 
               + ", confidence:" + std::to_string(confidence) 
               + ", uncertainty:" + std::to_string(uncertainty) + ")";
    }
};

// Guardrail 触发类型
enum class GuardrailType {
    NONE = 0,
    HIGH_RISK = 1,           // missing_risk > 0.9
    ILLEGAL_LOCATION = 2,    // in_illegal_location
    LOW_CONFIDENCE = 3       // bandit confidence < 0.6
};

struct GuardrailDecision {
    Action action;
    GuardrailType type;
    std::string reason;
    bool triggered;
    
    std::string getTypeString() const {
        switch (type) {
            case GuardrailType::HIGH_RISK: return "HIGH_RISK_GUARDRAIL";
            case GuardrailType::ILLEGAL_LOCATION: return "ILLEGAL_LOCATION_GUARDRAIL";
            case GuardrailType::LOW_CONFIDENCE: return "LOW_CONFIDENCE_GUARDRAIL";
            default: return "NONE";
        }
    }
};

// 动态行动成本惩罚（风险敏感）- v3.2.5
inline double getActionCost(ActionType type, double risk = 0.5) {
    double base_cost = 0.0;
    switch (type) {
        case ActionType::INSPECT:     base_cost = 0.3; break;
        case ActionType::ALERT:       base_cost = 0.2; break;
        case ActionType::REALLOCATE:  base_cost = 0.4; break;
        case ActionType::NO_ACTION:   base_cost = 0.0; break;
        default: base_cost = 0.0; break;
    }

    // 动态调整：高风险时，高成本动作的权重降低
    if (risk > 0.8) {
        base_cost *= 0.3;  // 高风险时，检查成本降 70%
    } else if (risk > 0.5) {
        base_cost *= 0.6;  // 中风险时，检查成本降 40%
    }

    return base_cost;
}

// Action 分布保护 - Soft Constraint（v3.3）
inline double getDistributionPenalty(double inspect_ratio, double no_action_ratio, double target_inspect = 0.2) {
    double penalty = 0.0;
    // Soft constraint: 行为分布引导（不是强制）
    penalty = -0.1 * std::abs(inspect_ratio - target_inspect);
    // 仍然保留 NO_ACTION 上限保护
    if (no_action_ratio > 0.85) penalty -= 0.15;
    return penalty;
}

// 延迟奖励结构
struct DelayedReward {
    int decision_id;
    double reward;
    int delay_hours;
    std::string status;  // pending, processed, expired
};

// LinUCB 臂模型
struct ArmModel {
    Action action;
    Eigen::MatrixXd A;  // d x d 特征矩阵
    Eigen::VectorXd b;  // d x 1 奖励向量
    int sample_count;   // 采样次数
    
    ArmModel(int dim, const Action& act) 
        : action(act), 
          A(Eigen::MatrixXd::Identity(dim, dim) * 0.01),
          b(Eigen::VectorXd::Zero(dim)),
          sample_count(0) {}
};

// Bandit 日志记录
struct BanditLog {
    int decision_id;
    ContextFeatures context;
    Action action;
    double reward;
    std::string mode;  // SHADOW, SUGGESTION, AUTO
};

// 运行模式
enum class BanditMode {
    SHADOW,       // 影子模式：只预测，不影响决策
    SUGGESTION,   // 建议模式：显示 Bandit 推荐 vs Rule 推荐
    AUTO          // 自动模式：Bandit 覆盖 DecisionEngine
};

} // namespace bandit