#pragma once
#include <string>

namespace monitoring {

class HealthMonitor {
public:
    static HealthMonitor& instance();

    void report(
        const std::string& component,
        const std::string& status,
        int latency_ms = 0,
        const std::string& error = ""
    );

private:
    HealthMonitor() = default;
    HealthMonitor(const HealthMonitor&) = delete;
    HealthMonitor& operator=(const HealthMonitor&) = delete;
};

} // namespace monitoring
