#include "proof_verification_gate.h"

namespace runtime {

ProofVerificationGate::ProofVerificationGate()
    : proof_engine_(&SystemProofEngine::instance()) {
}

bool ProofVerificationGate::verify(uint64_t frame_id) {
    last_result_ = proof_engine_->proveFrame(frame_id);
    return last_result_.valid;
}

const ProofResult& ProofVerificationGate::getLastResult() const {
    return last_result_;
}

double ProofVerificationGate::getConfidence() const {
    return last_result_.confidence;
}

} // namespace runtime