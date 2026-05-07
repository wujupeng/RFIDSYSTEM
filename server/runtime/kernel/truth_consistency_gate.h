#pragma once

#include "../system_consistency_checker.h"

namespace runtime {

/**
 * ============================
 * Truth Consistency Gate
 * ============================
 *
 * 职责：验证系统三层一致性
 * - Frame一致性
 * - AI一致性
 * - GPU一致性
 *
 * 输出：可信度分数 (0-100)
 */
class TruthConsistencyGate {
public:
    TruthConsistencyGate();

    /**
     * 执行一致性验证
     */
    int evaluate(const ConsistencyInput& input);

    /**
     * 获取最后一次验证结果
     */
    const ConsistencyResult& getLastResult() const;

    /**
     * 是否通过验证
     */
    bool isValid() const;

private:
    SystemConsistencyChecker checker_;
    ConsistencyResult last_result_;
    int last_score_ = 0;
};

} // namespace runtime