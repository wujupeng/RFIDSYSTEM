#include "drift_degradation_chain.h"

namespace pa::spatial_truth {

DriftDegradationChain& DriftDegradationChain::instance() {
    static DriftDegradationChain inst;
    return inst;
}

void DriftDegradationChain::onTruthGateFailed(const TruthGateResult& result) {
    if (result.pass) {
        reset();
        return;
    }

    consecutive_fail_count_.fetch_add(1);

    if (consecutive_fail_count_.load() >= fail_threshold_) {
        triggerDegradation();
    }
}

void DriftDegradationChain::reset() {
    consecutive_fail_count_.store(0);
}

void DriftDegradationChain::triggerDegradation() {
    triggerRecalibration();
    triggerAutoTuning();
}

void DriftDegradationChain::triggerRecalibration() {
}

void DriftDegradationChain::triggerAutoTuning() {
}

} // namespace pa::spatial_truth