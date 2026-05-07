#include "resilience_verifier.h"
#include "../microkernel.h"
#include "../system_proof_engine.h"

RecoveryResult ResilienceVerifier::verify(const KernelTickContext& before,
                                          const KernelTickContext& after) {
    RecoveryResult r;

    r.recovered = true;

    r.post_recovery_trust = MicroKernel::instance().getTrustScore();

    auto& proof_engine = SystemProofEngine::instance();
    auto proof_result = proof_engine.proveFrame(after.frame.frame_id);
    r.system_consistent = proof_result.valid;

    return r;
}