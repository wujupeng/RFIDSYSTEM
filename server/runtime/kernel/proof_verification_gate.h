#pragma once

#include "../system_proof_engine.h"

namespace runtime {

/**
 * ============================
 * Proof Verification Gate
 * ============================
 *
 * 职责：验证系统可证明性
 * - Deterministic Proof
 * - Causal Proof
 * - Recovery Proof
 */
class ProofVerificationGate {
public:
    ProofVerificationGate();

    /**
     * 执行可证明性验证
     */
    bool verify(uint64_t frame_id);

    /**
     * 获取最后一次验证结果
     */
    const ProofResult& getLastResult() const;

    /**
     * 获取置信度
     */
    double getConfidence() const;

private:
    SystemProofEngine* proof_engine_;
    ProofResult last_result_;
};

} // namespace runtime