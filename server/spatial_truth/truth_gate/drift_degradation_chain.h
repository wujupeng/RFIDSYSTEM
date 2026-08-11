#pragma once

#include "truth_gate.h"
#include <atomic>
#include <string>

namespace pa::spatial_truth {

class DriftDegradationChain {
public:
    static DriftDegradationChain& instance();

    void onTruthGateFailed(const TruthGateResult& result);
    int getConsecutiveFailCount() const { return consecutive_fail_count_.load(); }
    void reset();

    void setConsecutiveFailThreshold(int threshold) { fail_threshold_ = threshold; }

private:
    DriftDegradationChain() = default;

    std::atomic<int> consecutive_fail_count_{0};
    int fail_threshold_ = 3;

    void triggerDegradation();
    void triggerRecalibration();
    void triggerAutoTuning();
};

} // namespace pa::spatial_truth