#include "metrics.h"
#include "../repository/metrics_repository.h"

namespace monitoring {

void Metrics::gauge(const std::string& name, double value) {
    repository::MetricsRepository::instance().insert(name, value);
}

void Metrics::increment(const std::string& name, double value) {
    repository::MetricsRepository::instance().insert(name, value);
}

void Metrics::gauge(const std::string& name, double value, const nlohmann::json& tags) {
    repository::MetricsRepository::instance().insert(name, value, tags);
}

void Metrics::increment(const std::string& name, double value, const nlohmann::json& tags) {
    repository::MetricsRepository::instance().insert(name, value, tags);
}

} // namespace monitoring
