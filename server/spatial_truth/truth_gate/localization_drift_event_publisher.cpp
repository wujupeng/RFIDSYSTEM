#include "localization_drift_event_publisher.h"
#include <chrono>

namespace pa::spatial_truth {

bool LocalizationDriftEventPublisher::publish(const TruthGateResult& result, uint64_t frame_id) {
    if (result.pass) return true;

    FailureEvent event;
    event.type = FailureType::SPATIAL_LOCALIZATION_DRIFT;
    event.frame_id = frame_id;
    event.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    event.message = result.reason;
    event.severity = 0.8;
    event.acknowledged = false;
    if (!result.tag_id.empty()) {
        event.tag_id = result.tag_id;
    }
    event.error_value = result.error_value;
    event.threshold = result.threshold;

    try {
        RuntimeFailureBus::instance().publish(event);
        return true;
    } catch (...) {
        cached_events_.push_back(event);
        return false;
    }
}

} // namespace pa::spatial_truth