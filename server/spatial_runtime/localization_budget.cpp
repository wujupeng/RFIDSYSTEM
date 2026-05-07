#include "localization_budget.h"

LocalizationBudget::LocalizationBudget() {
    budgets_["rssi"] = {"rssi", 5.0, 0.0, true};
    budgets_["phase"] = {"phase", 5.0, 0.0, true};
    budgets_["aoa"] = {"aoa", 10.0, 0.0, true};
    budgets_["triangulation"] = {"triangulation", 5.0, 0.0, true};
    budgets_["kalman"] = {"kalman", 5.0, 0.0, true};
    budgets_["beamforming"] = {"beamforming", 20.0, 0.0, true};
}

LocalizationBudget& LocalizationBudget::instance() {
    static LocalizationBudget instance;
    return instance;
}

void LocalizationBudget::reset() {
    for (auto& pair : budgets_) {
        pair.second.current_ms = 0.0;
    }
}

void LocalizationBudget::recordTime(const std::string& module, double ms) {
    auto it = budgets_.find(module);
    if (it != budgets_.end()) {
        it->second.current_ms += ms;
    }
}

bool LocalizationBudget::hasBudget(const std::string& module) {
    auto it = budgets_.find(module);
    if (it == budgets_.end()) {
        return false;
    }
    return it->second.enabled && it->second.current_ms < it->second.max_ms;
}

double LocalizationBudget::getRemaining(const std::string& module) {
    auto it = budgets_.find(module);
    if (it == budgets_.end()) {
        return 0.0;
    }
    return std::max(0.0, it->second.max_ms - it->second.current_ms);
}

void LocalizationBudget::disableModule(const std::string& module) {
    auto it = budgets_.find(module);
    if (it != budgets_.end()) {
        it->second.enabled = false;
    }
}

void LocalizationBudget::enableModule(const std::string& module) {
    auto it = budgets_.find(module);
    if (it != budgets_.end()) {
        it->second.enabled = true;
    }
}

bool LocalizationBudget::isOverBudget() const {
    for (const auto& pair : budgets_) {
        if (pair.second.enabled && pair.second.current_ms > pair.second.max_ms) {
            return true;
        }
    }
    return false;
}

double LocalizationBudget::getTotalTime() const {
    double total = 0.0;
    for (const auto& pair : budgets_) {
        total += pair.second.current_ms;
    }
    return total;
}