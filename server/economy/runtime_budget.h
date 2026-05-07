#pragma once

#include <cstdint>
#include <string>
#include <map>

struct BudgetAllocation {
    std::string resource_type;
    double allocated;
    double used;
    double limit;
};

class RuntimeBudget {
public:
    static RuntimeBudget& instance();
    
    bool allocateResource(const std::string& resource_type, double amount);
    
    void releaseResource(const std::string& resource_type, double amount);
    
    double getRemaining(const std::string& resource_type);
    
    bool hasBudget(const std::string& resource_type, double amount);
    
    void setLimit(const std::string& resource_type, double limit);
    
    std::map<std::string, BudgetAllocation> getAllAllocations();
    
private:
    RuntimeBudget();
    
    std::map<std::string, BudgetAllocation> allocations_;
};