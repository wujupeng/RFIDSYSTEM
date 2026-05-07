#pragma once

#include "../system_consistency_checker.h"
#include "../proof_ledger.h"
#include "../system_proof_engine.h"
#include "../runtime_recovery_engine.h"

namespace runtime {

/**
 * ============================
 * Runtime Kernel（系统内核）
 * ============================
 *
 * 职责：
 * 1. Consistency Gate（真实性判断）
 * 2. Proof Gate（可证明性验证）
 * 3. Causal Gate（因果链验证）
 * 4. Self-Healing（自动恢复）
 *
 * 👉 这是整个系统的"总调度器"
 */
class RuntimeKernel {
public:
    RuntimeKernel();

    /**
     * 每帧进入 kernel
     */
    void tick(const ConsistencyInput& input);

    /**
     * 当前系统是否可信
     */
    bool isSystemTrusted() const;

    /**
     * 获取信任分数
     */
    int getTrustScore() const;

    /**
     * 重置信任分数
     */
    void resetTrustScore();

private:
    SystemConsistencyChecker checker_;
    ProofLedger* ledger_;
    SystemProofEngine* proof_engine_;
    RuntimeRecoveryEngine* recovery_engine_;

    int trust_score_ = 100;

    void evaluateConsistency(const ConsistencyInput& input);
    void evaluateProof(uint64_t frame_id);
    void evaluateCausality(uint64_t frame_id);
    void triggerHealingIfNeeded();
};

} // namespace runtime