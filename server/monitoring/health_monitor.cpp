#include "health_monitor.h"
#include "../repository/health_repository.h"

namespace monitoring {

HealthMonitor& HealthMonitor::instance() {
    static HealthMonitor instance;
    return instance;
}

void HealthMonitor::report(
    const std::string& component,
    const std::string& status,
    int latency_ms,
    const std::string& error)
{
    repository::HealthRepository::instance().insert(component, status, latency_ms, error);
}

} // namespace monitoring
