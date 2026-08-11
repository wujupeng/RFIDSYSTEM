#pragma once

#include "truth_gate.h"
#include "drift_degradation_chain.h"
#include "localization_drift_event_publisher.h"
#include <atomic>
#include <string>

namespace pa::spatial_truth {

class SpatialSelfHealingLoop {
public:
    static SpatialSelfHealingLoop& instance();

    void run();
    bool isClosed() const { return closed_.load(); }

private:
    SpatialSelfHealingLoop() = default;

    std::atomic<bool> closed_{false};
    LocalizationDriftEventPublisher event_publisher_;
};

} // namespace pa::spatial_truth