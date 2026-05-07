#pragma once

#include "../runtime_recovery_engine.h"
#include "../system_consistency_checker.h"

namespace runtime {

/**
 * ============================
 * Self Healing Controller
 * ============================
 *
 * 职责：根据系统状态触发自动恢复
 * - 监控 trust score
 * - 决定恢复策略
 * - 执行恢复动作
 */
class SelfHealingController {
public:
    SelfHealingController();

    /**
     * 更新系统状态并决定是否恢复
     */
    void update(int trust_score, const ConsistencyResult& consistency);

    /**
     * 是否正在恢复中
     */
    bool isRecovering() const;

    /**
     * 获取最后执行的恢复动作
     */
    RecoveryAction getLastAction() const;

    /**
     * 重置恢复状态
     */
    void reset();

private:
    RuntimeRecoveryEngine* recovery_engine_;
    bool is_recovering_ = false;
    RecoveryAction last_action_ = RecoveryAction::NONE;

    void triggerRecovery(int trust_score);
};

} // namespace runtime