#pragma once
#include <string>
#include <nlohmann/json.hpp>

namespace monitoring {

class Metrics {
public:
    static void gauge(const std::string& name, double value);
    static void increment(const std::string& name, double value = 1.0);
    static void gauge(const std::string& name, double value, const nlohmann::json& tags);
    static void increment(const std::string& name, double value, const nlohmann::json& tags);
};

} // namespace monitoring
