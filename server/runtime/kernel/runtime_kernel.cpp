#include "runtime_kernel.h"

namespace runtime {

RuntimeKernel::RuntimeKernel()
    : ledger_(&ProofLedger::instance()),
      proof_engine_(&SystemProofEngine::instance()),
      recovery_engine_(&RuntimeRecoveryEngine::instance()) {
}

void RuntimeKernel::tick(const ConsistencyInput& input) {

    // =========================
    // 1. Consistency Gate
    // =========================
    evaluateConsistency(input);

    // =========================
    // 2. Proof Gate
    // =========================
    evaluateProof(input.frame_id);

    // =========================
    // 3. Causal Gate
    // =========================
    evaluateCausality(input.frame_id);

    // =========================
    // 4. Self-Healing Gate
    // =========================
    triggerHealingIfNeeded();
}

// -------------------------
// 1. Consistency
// -------------------------
void RuntimeKernel::evaluateConsistency(const ConsistencyInput& input) {

    auto result = checker_.verify(input);

    if (!result.frame_consistent) trust_score_ -= 20;
    if (!result.ai_consistent)    trust_score_ -= 30;
    if (!result.gpu_consistent)   trust_score_ -= 30;

    if (result.confidence < 0.6) {
        trust_score_ -= 10;
    }

    trust_score_ = std::max(0, std::min(100, trust_score_));
}

// -------------------------
// 2. Proof Layer
// -------------------------
void RuntimeKernel::evaluateProof(uint64_t frame_id) {

    ProofResult result = proof_engine_->proveFrame(frame_id);

    if (!result.valid) {
        trust_score_ -= 25;
    }

    if (!result.deterministic_ok) {
        trust_score_ -= 25;
    }

    if (!result.causal_ok) {
        trust_score_ -= 20;
    }

    trust_score_ = std::max(0, std::min(100, trust_score_));
}

// -------------------------
// 3. Causal Layer
// -------------------------
void RuntimeKernel::evaluateCausality(uint64_t frame_id) {

    trust_score_ = std::max(0, std::min(100, trust_score_));
}

// -------------------------
// 4. Self Healing
// -------------------------
void RuntimeKernel::triggerHealingIfNeeded() {

    if (trust_score_ > 80) return;

    if (trust_score_ < 50) {
        recovery_engine_->execute(RecoveryAction::SWITCH_TO_SAFE_POLICY);
    }

    if (trust_score_ < 30) {
        recovery_engine_->execute(RecoveryAction::RESTART_SUBSYSTEM);
    }

    if (trust_score_ < 10) {
        recovery_engine_->execute(RecoveryAction::EMERGENCY_STOP);
    }
}

// -------------------------
bool RuntimeKernel::isSystemTrusted() const {
    return trust_score_ > 70;
}

int RuntimeKernel::getTrustScore() const {
    return trust_score_;
}

void RuntimeKernel::resetTrustScore() {
    trust_score_ = 100;
}

} // namespace runtime