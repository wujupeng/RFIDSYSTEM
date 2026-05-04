#include "parameter_space.h"
#include <algorithm>

ParameterSpace& ParameterSpace::instance() {
    static ParameterSpace instance;
    return instance;
}

ParameterSpace::ParameterSpace() {
    initializeDefaults();
}

void ParameterSpace::initializeDefaults() {
    parameters_ = {
        {"missing_hours_threshold", 24.0, 168.0, 6.0, 72.0, 72.0},
        {"inactivity_hours_threshold", 12.0, 120.0, 4.0, 48.0, 48.0},
        {"abnormal_score_threshold", 0.3, 0.95, 0.05, 0.8, 0.8},
        {"cooldown_alert_minutes", 30.0, 120.0, 10.0, 60.0, 60.0},
        {"cooldown_inspect_minutes", 15.0, 60.0, 5.0, 30.0, 30.0},
        {"cooldown_noaction_minutes", 5.0, 30.0, 5.0, 15.0, 15.0},
        {"missing_risk_weight", 0.1, 0.8, 0.05, 0.4, 0.4},
        {"inactivity_risk_weight", 0.1, 0.6, 0.05, 0.3, 0.3},
        {"abnormal_risk_weight", 0.1, 0.5, 0.05, 0.3, 0.3},
        {"alert_priority_threshold", 1.0, 4.0, 0.5, 2.0, 2.0},
        {"inspect_priority_threshold", 2.0, 5.0, 0.5, 3.5, 3.5}
    };
}

std::vector<TunableParameter> ParameterSpace::getAllParameters() const {
    return parameters_;
}

void ParameterSpace::updateParameter(const std::string& name, double value) {
    auto it = std::find_if(parameters_.begin(), parameters_.end(),
        [&name](const TunableParameter& p) { return p.name == name; });

    if (it != parameters_.end()) {
        it->current_value = std::max(it->min_value, std::min(it->max_value, value));
    }
}

std::unordered_map<std::string, double> ParameterSpace::snapshot() {
    std::unordered_map<std::string, double> result;
    for (const auto& p : parameters_) {
        result[p.name] = p.current_value;
    }
    return result;
}

void ParameterSpace::restore(const std::unordered_map<std::string, double>& params) {
    for (const auto& [name, value] : params) {
        updateParameter(name, value);
    }
}

double ParameterSpace::getParameter(const std::string& name) const {
    auto it = std::find_if(parameters_.begin(), parameters_.end(),
        [&name](const TunableParameter& p) { return p.name == name; });

    if (it != parameters_.end()) {
        return it->current_value;
    }
    return 0.0;
}

void ParameterSpace::resetToDefaults() {
    for (auto& p : parameters_) {
        p.current_value = p.default_value;
    }
}
