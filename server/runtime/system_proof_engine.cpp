#include "system_proof_engine.h"
#include "proof_ledger.h"
#include "system_consistency_checker.h"

SystemProofEngine::SystemProofEngine() {
}

SystemProofEngine& SystemProofEngine::instance() {
    static SystemProofEngine instance;
    return instance;
}

void SystemProofEngine::initialize() {
}

void SystemProofEngine::shutdown() {
}

SystemProofEngine::ProofResult SystemProofEngine::proveFrame(uint64_t frameId) {
    ProofResult result;
    
    std::vector<std::string> violations;
    
    bool det_ok = verifyDeterminism(frameId);
    bool causal_ok = verifyCausality(frameId);
    
    result.deterministic_ok = det_ok;
    result.causal_ok = causal_ok;
    result.recovery_ok = true;
    
    if (!det_ok) {
        violations.push_back("Determinism violation in frame " + std::to_string(frameId));
    }
    
    if (!causal_ok) {
        violations.push_back("Causality violation in frame " + std::to_string(frameId));
    }
    
    result.valid = det_ok && causal_ok;
    result.violations = violations;
    
    double score = 0.0;
    if (det_ok) score += 0.4;
    if (causal_ok) score += 0.4;
    score += 0.2;
    
    result.confidence = score;
    
    return result;
}

SystemProofEngine::ProofResult SystemProofEngine::proveAI(uint64_t frameId) {
    ProofResult result;
    
    std::vector<std::string> violations;
    
    bool det_ok = verifyDeterminism(frameId);
    bool causal_ok = verifyCausality(frameId);
    
    result.deterministic_ok = det_ok;
    result.causal_ok = causal_ok;
    result.recovery_ok = true;
    
    if (!det_ok) {
        violations.push_back("AI determinism violation in frame " + std::to_string(frameId));
    }
    
    if (!causal_ok) {
        violations.push_back("AI causality violation in frame " + std::to_string(frameId));
    }
    
    result.valid = det_ok && causal_ok;
    result.violations = violations;
    
    double score = 0.0;
    if (det_ok) score += 0.5;
    if (causal_ok) score += 0.5;
    
    result.confidence = score;
    
    return result;
}

SystemProofEngine::ProofResult SystemProofEngine::proveRecovery(uint64_t frameId) {
    ProofResult result;
    
    std::vector<std::string> violations;
    
    bool recovery_ok = verifyRecoveryState(frameId);
    
    result.deterministic_ok = true;
    result.causal_ok = true;
    result.recovery_ok = recovery_ok;
    
    if (!recovery_ok) {
        violations.push_back("Recovery state violation in frame " + std::to_string(frameId));
    }
    
    result.valid = recovery_ok;
    result.violations = violations;
    result.confidence = recovery_ok ? 1.0 : 0.0;
    
    return result;
}

SystemProofEngine::ProofResult SystemProofEngine::proveSystemHealth() {
    ProofResult result;
    
    std::vector<std::string> violations;
    
    bool det_ok = verifyDeterminism(0);
    bool causal_ok = verifyCausality(0);
    bool recovery_ok = verifyRecoveryState(0);
    
    result.deterministic_ok = det_ok;
    result.causal_ok = causal_ok;
    result.recovery_ok = recovery_ok;
    
    if (!det_ok) {
        violations.push_back("System-wide determinism violation");
    }
    
    if (!causal_ok) {
        violations.push_back("System-wide causality violation");
    }
    
    if (!recovery_ok) {
        violations.push_back("System-wide recovery state violation");
    }
    
    result.valid = det_ok && causal_ok && recovery_ok;
    result.violations = violations;
    
    double score = 0.0;
    if (det_ok) score += 0.33;
    if (causal_ok) score += 0.33;
    if (recovery_ok) score += 0.34;
    
    result.confidence = score;
    
    return result;
}

bool SystemProofEngine::verifyDeterminism(uint64_t frameId) {
    auto& checker = SystemConsistencyChecker::instance();
    return checker.isConsistent();
}

bool SystemProofEngine::verifyCausality(uint64_t frameId) {
    auto& ledger = ProofLedger::instance();
    auto records = ledger.query(frameId);
    
    if (records.empty()) {
        return false;
    }
    
    const auto& record = records[0];
    
    if (record.frame_hash == 0 || record.ai_hash == 0) {
        return false;
    }
    
    return true;
}

bool SystemProofEngine::verifyRecoveryState(uint64_t frameId) {
    auto& ledger = ProofLedger::instance();
    auto records = ledger.query(frameId);
    
    if (records.empty()) {
        return false;
    }
    
    const auto& record = records[0];
    
    return record.recovery_hash != 0;
}