#pragma once

#include <cstdint>
#include <string>
#include <map>

struct ModuleBudget {
    std::string name;
    double max_ms;
    double current_ms;
    bool enabled;
};

class LocalizationBudget {
public:
    static LocalizationBudget& instance();
    
    void reset();
    
    void recordTime(const std::string& module, double ms);
    
    bool hasBudget(const std::string& module);
    
    double getRemaining(const std::string& module);
    
    void disableModule(const std::string& module);
    
    void enableModule(const std::string& module);
    
    bool isOverBudget() const;
    
    double getTotalTime() const;
    
private:
    LocalizationBudget();
    
    std::map<std::string, ModuleBudget> budgets_;
};