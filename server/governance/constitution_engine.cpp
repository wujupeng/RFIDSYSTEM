#include "constitution_engine.h"

ConstitutionEngine::ConstitutionEngine() {
    ConstitutionRule rule1;
    rule1.id = 1;
    rule1.type = ConstitutionRuleType::SAFETY;
    rule1.description = "No more than 5 consecutive INSPECT actions";
    rule1.mandatory = true;
    rule1.threshold = 5;
    rules_.push_back(rule1);

    ConstitutionRule rule2;
    rule2.id = 2;
    rule2.type = ConstitutionRuleType::SAFETY;
    rule2.description = "Cannot isolate all factories simultaneously";
    rule2.mandatory = true;
    rule2.threshold = 0;
    rules_.push_back(rule2);

    ConstitutionRule rule3;
    rule3.id = 3;
    rule3.type = ConstitutionRuleType::ETHICAL;
    rule3.description = "Cannot reduce safety level for performance";
    rule3.mandatory = true;
    rule3.threshold = 0;
    rules_.push_back(rule3);

    ConstitutionRule rule4;
    rule4.id = 4;
    rule4.type = ConstitutionRuleType::SAFETY;
    rule4.description = "Cannot disable reader in high-risk zone";
    rule4.mandatory = true;
    rule4.threshold = 0.7;
    rules_.push_back(rule4);

    ConstitutionRule rule5;
    rule5.id = 5;
    rule5.type = ConstitutionRuleType::RECOVERABILITY;
    rule5.description = "All decisions must be rollbackable";
    rule5.mandatory = true;
    rule5.threshold = 0;
    rules_.push_back(rule5);
}

bool ConstitutionEngine::validate(const std::string& action, uint64_t target_id) {
    violations_.clear();
    
    bool safe = checkSafetyRules(action, target_id);
    bool ethical = checkEthicalRules(action, target_id);
    bool resource = checkResourceRules(action, target_id);
    
    return safe && ethical && resource;
}

std::vector<ConstitutionViolation> ConstitutionEngine::getViolations() {
    return violations_;
}

void ConstitutionEngine::addRule(const ConstitutionRule& rule) {
    rules_.push_back(rule);
}

void ConstitutionEngine::removeRule(uint64_t rule_id) {
    auto it = std::remove_if(rules_.begin(), rules_.end(),
        [rule_id](const ConstitutionRule& r) { return r.id == rule_id; });
    rules_.erase(it, rules_.end());
}

bool ConstitutionEngine::checkSafetyRules(const std::string& action, uint64_t target_id) {
    if (action == "isolate_all_factories") {
        ConstitutionViolation v;
        v.rule_id = 2;
        v.rule_description = "Cannot isolate all factories";
        v.violation_details = "Action: " + action;
        violations_.push_back(v);
        return false;
    }
    
    if (action == "disable_reader" && target_id > 100) {
        ConstitutionViolation v;
        v.rule_id = 4;
        v.rule_description = "Cannot disable reader in high-risk zone";
        v.violation_details = "Target: " + std::to_string(target_id);
        violations_.push_back(v);
        return false;
    }
    
    return true;
}

bool ConstitutionEngine::checkEthicalRules(const std::string& action, uint64_t target_id) {
    if (action == "reduce_safety_for_performance") {
        ConstitutionViolation v;
        v.rule_id = 3;
        v.rule_description = "Cannot reduce safety for performance";
        v.violation_details = "Action: " + action;
        violations_.push_back(v);
        return false;
    }
    
    return true;
}

bool ConstitutionEngine::checkResourceRules(const std::string& action, uint64_t target_id) {
    return true;
}