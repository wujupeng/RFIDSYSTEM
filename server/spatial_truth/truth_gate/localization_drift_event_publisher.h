#pragma once

#include "truth_gate.h"
#include "../../runtime/runtime_failure_bus.h"
#include <string>

namespace pa::spatial_truth {

class LocalizationDriftEventPublisher {
public:
    bool publish(const TruthGateResult& result, uint64_t frame_id);

private:
    std::vector<FailureEvent> cached_events_;
};

} // namespace pa::spatial_truth