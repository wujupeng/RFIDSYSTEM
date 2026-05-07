#pragma once

#include <cstdint>
#include <string>
#include <vector>

enum class ConstitutionRuleType {
    SAFETY,
    RECOVERABILITY,
    EXPLAINABILITY,
    ETHICAL,
    RESOURCE_LIMIT
};

struct ConstitutionRule {
    uint64_t id;
    ConstitutionRuleType type;
    std::string description;
    bool mandatory;
    double threshold;
};

struct ConstitutionViolation {
    uint64_t rule_id;
    std::string rule_description;
    std::string violation_details;
};

class ConstitutionEngine {
public:
    ConstitutionEngine();
    
    bool validate(const std::string& action, uint64_t target_id);
    
    std::vector<ConstitutionViolation> getViolations();
    
    void addRule(const ConstitutionRule& rule);
    
    void removeRule(uint64_t rule_id);
    
private:
    bool checkSafetyRules(const std::string& action, uint64_t target_id);
    bool checkEthicalRules(const std::string& action, uint64_t target_id);
    bool checkResourceRules(const std::string& action, uint64_t target_id);
    
    std::vector<ConstitutionRule> rules_;
    std::vector<ConstitutionViolation> violations_;
};