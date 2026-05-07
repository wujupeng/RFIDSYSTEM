#include "microkernel.h"
#include "kernel_pipeline.h"

MicroKernel& MicroKernel::instance() {
    static MicroKernel k;
    return k;
}

KernelTickResult MicroKernel::tick(const KernelTickContext& ctx) {

    KernelTickResult result;

    // =========================
    // 1️⃣ Consistency Gate
    // =========================
    bool consistency_ok = KernelPipeline::checkConsistency(ctx);

    // =========================
    // 2️⃣ Proof Gate
    // =========================
    bool proof_ok = KernelPipeline::verifyProof(ctx);

    // =========================
    // 3️⃣ Trust Computation
    // =========================
    if (!consistency_ok) trust_score_ -= 20;
    if (!proof_ok) trust_score_ -= 15;

    trust_score_ = std::max(0, std::min(100, trust_score_));

    if (trust_score_ > 80)
        result.abi.mode = KernelMode::NORMAL;
    else if (trust_score_ > 50)
        result.abi.mode = KernelMode::DEGRADED;
    else if (trust_score_ > 30)
        result.abi.mode = KernelMode::SAFE;
    else if (trust_score_ > 10)
        result.abi.mode = KernelMode::INSPECT;
    else
        result.abi.mode = KernelMode::EMERGENCY;

    result.abi.trust_score = trust_score_;

    // =========================
    // 4️⃣ Healing Decision
    // =========================
    result.action = KernelPipeline::heal(ctx);

    // =========================
    // 5️⃣ ABI Freeze Output
    // =========================
    result.abi.deterministic_ok = consistency_ok;
    result.abi.causal_ok = proof_ok;
    result.abi.recovery_ok = true;

    result.abi.frame_id = ctx.frame.frame_id;
    result.abi.timestamp = ctx.frame.timestamp;

    return result;
}

int MicroKernel::getTrustScore() const {
    return trust_score_;
}

void MicroKernel::resetTrustScore() {
    trust_score_ = 100;
}