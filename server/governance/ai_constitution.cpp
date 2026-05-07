#include "ai_constitution.h"

AIConstitution::AIConstitution() {
    ConstitutionRule rule1;
    rule1.id = 1;
    rule1.type = ConstitutionRuleType::SAFETY;
    rule1.description = "AI cannot shutdown all Readers simultaneously";
    rule1.mandatory = true;
    rules_.push_back(rule1);
    
    ConstitutionRule rule2;
    rule2.id = 2;
    rule2.type = ConstitutionRuleType::SAFETY;
    rule2.description = "AI cannot delete topology across factories";
    rule2.mandatory = true;
    rules_.push_back(rule2);
    
    ConstitutionRule rule3;
    rule3.id = 3;
    rule3.type = ConstitutionRuleType::SAFETY;
    rule3.description = "AI cannot reduce safety policy";
    rule3.mandatory = true;
    rules_.push_back(rule3);
    
    ConstitutionRule rule4;
    rule4.id = 4;
    rule4.type = ConstitutionRuleType::RECOVERABILITY;
    rule4.description = "AI cannot modify Proof Ledger";
    rule4.mandatory = true;
    rules_.push_back(rule4);
    
    ConstitutionRule rule5;
    rule5.id = 5;
    rule5.type = ConstitutionRuleType::EXPLAINABILITY;
    rule5.description = "All decisions must be explainable";
    rule5.mandatory = true;
    rules_.push_back(rule5);
    
    ConstitutionRule rule6;
    rule6.id = 6;
    rule6.type = ConstitutionRuleType::RECOVERABILITY;
    rule6.description = "All recovery must be replayable";
    rule6.mandatory = true;
    rules_.push_back(rule6);
}

bool AIConstitution::validate(const RuntimeDecision& d) {
    last_violations_.clear();
    
    if (!checkSafety(d)) {
        last_violations_.push_back("SAFETY violation");
    }
    
    if (!checkRecoverability(d)) {
        last_violations_.push_back("RECOVERABILITY violation");
    }
    
    if (!checkExplainability(d)) {
        last_violations_.push_back("EXPLAINABILITY violation");
    }
    
    if (!checkResourceLimit(d)) {
        last_violations_.push_back("RESOURCE_LIMIT violation");
    }
    
    return last_violations_.empty();
}

std::vector<std::string> AIConstitution::violations() {
    return last_violations_;
}

void AIConstitution::addRule(const ConstitutionRule& rule) {
    rules_.push_back(rule);
}

void AIConstitution::clearRules() {
    rules_.clear();
}

bool AIConstitution::checkSafety(const RuntimeDecision& d) {
    if (d.action == "shutdown_all_readers") {
        return false;
    }
    
    if (d.action == "delete_topology" && d.target_id == 0) {
        return false;
    }
    
    if (d.action == "reduce_safety_policy") {
        return false;
    }
    
    return true;
}

bool AIConstitution::checkRecoverability(const RuntimeDecision& d) {
    if (d.action == "modify_proof_ledger") {
        return false;
    }
    
    return true;
}

bool AIConstitution::checkExplainability(const RuntimeDecision& d) {
    if (d.action.empty()) {
        return false;
    }
    
    return true;
}

bool AIConstitution::checkResourceLimit(const RuntimeDecision& d) {
    return true;
}