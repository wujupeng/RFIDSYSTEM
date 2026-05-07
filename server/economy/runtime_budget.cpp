#include "runtime_budget.h"

RuntimeBudget::RuntimeBudget() {
    allocations_["gpu_memory"] = {"gpu_memory", 0, 0, 4096};
    allocations_["compute_units"] = {"compute_units", 0, 0, 100};
    allocations_["bandwidth"] = {"bandwidth", 0, 0, 1000};
    allocations_["ai_inference"] = {"ai_inference", 0, 0, 50};
}

RuntimeBudget& RuntimeBudget::instance() {
    static RuntimeBudget instance;
    return instance;
}

bool RuntimeBudget::allocateResource(const std::string& resource_type, double amount) {
    auto it = allocations_.find(resource_type);
    if (it == allocations_.end()) {
        return false;
    }
    
    if (it->second.used + amount > it->second.limit) {
        return false;
    }
    
    it->second.used += amount;
    return true;
}

void RuntimeBudget::releaseResource(const std::string& resource_type, double amount) {
    auto it = allocations_.find(resource_type);
    if (it != allocations_.end()) {
        it->second.used = std::max(0.0, it->second.used - amount);
    }
}

double RuntimeBudget::getRemaining(const std::string& resource_type) {
    auto it = allocations_.find(resource_type);
    if (it == allocations_.end()) {
        return 0.0;
    }
    return it->second.limit - it->second.used;
}

bool RuntimeBudget::hasBudget(const std::string& resource_type, double amount) {
    return getRemaining(resource_type) >= amount;
}

void RuntimeBudget::setLimit(const std::string& resource_type, double limit) {
    allocations_[resource_type].limit = limit;
}

std::map<std::string, BudgetAllocation> RuntimeBudget::getAllAllocations() {
    return allocations_;
}