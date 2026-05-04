#pragma once

#include <string>
#include <vector>
#include <map>
#include <nlohmann/json.hpp>

struct RuleImpactResult {
    std::string rule_name;
    std::string rule_version;
    
    double impact_score = 0.0;
    double precision_delta = 0.0;
    double recall_delta = 0.0;
    double f1_delta = 0.0;
    
    double adoption_rate_delta = 0.0;
    double false_positive_delta = 0.0;
    double false_negative_delta = 0.0;
    
    int sample_size = 0;
    double confidence_interval = 0.0;
    
    std::string analysis_summary;
};

struct RuleImpactReport {
    std::string from_version;
    std::string to_version;
    
    std::vector<RuleImpactResult> impacts;
    
    double overall_accuracy_delta = 0.0;
    double overall_adoption_rate_delta = 0.0;
    
    std::string worst_performing_rule;
    std::string best_performing_rule;
    
    std::map<std::string, std::string> rule_changes_summary;
};

struct DecisionAttributionResult {
    int decision_id;
    int snapshot_id;
    
    std::string primary_rule;
    double primary_rule_contribution = 0.0;
    
    std::vector<std::pair<std::string, double>> contributing_rules;
    
    double attribution_confidence = 0.0;
};

class RuleImpactAnalyzer {
public:
    static RuleImpactAnalyzer& instance();

    RuleImpactReport analyzeVersionDiff(
        const std::string& from_version,
        const std::string& to_version,
        int lookback_hours = 24);

    DecisionAttributionResult analyzeDecisionAttribution(int decisionId);

    std::vector<RuleImpactResult> evaluateCurrentRules(int lookback_hours = 24);

    void storeImpactAnalysis(const RuleImpactResult& result);

    void storeDecisionAttribution(const DecisionAttributionResult& attribution);

private:
    RuleImpactAnalyzer() = default;

    double calculateRuleImpact(
        const std::string& ruleName,
        const std::string& version,
        int lookback_hours);

    std::vector<std::string> findChangedRules(
        const std::string& from_version,
        const std::string& to_version);

    double calculateAccuracyDelta(
        const std::string& from_version,
        const std::string& to_version,
        const std::string& ruleName);
};
