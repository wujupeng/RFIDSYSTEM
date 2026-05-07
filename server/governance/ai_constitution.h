#pragma once

#include <cstdint>
#include <string>
#include <vector>

enum class ConstitutionRuleType {
    SAFETY,
    RECOVERABILITY,
    EXPLAINABILITY,
    HUMAN_APPROVAL,
    RESOURCE_LIMIT
};

struct ConstitutionRule {
    uint64_t id;
    ConstitutionRuleType type;
    std::string description;
    bool mandatory;
};

struct RuntimeDecision {
    uint64_t decision_id;
    std::string action;
    uint64_t target_id;
    std::string payload;
    uint64_t timestamp;
};

class AIConstitution {
public:
    AIConstitution();
    
    bool validate(const RuntimeDecision& d);
    
    std::vector<std::string> violations();
    
    void addRule(const ConstitutionRule& rule);
    
    void clearRules();

private:
    bool checkSafety(const RuntimeDecision& d);
    bool checkRecoverability(const RuntimeDecision& d);
    bool checkExplainability(const RuntimeDecision& d);
    bool checkResourceLimit(const RuntimeDecision& d);
    
    std::vector<ConstitutionRule> rules_;
    std::vector<std::string> last_violations_;
};