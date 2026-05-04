#pragma once

#include <string>
#include <vector>
#include <optional>
#include <nlohmann/json.hpp>

struct RuleRecord {
    int id = 0;
    std::string rule_name;
    std::string rule_version;
    std::string rule_type;
    nlohmann::json rule_json;
    std::string description;
    bool enabled = true;
    std::string created_at;
};

struct RuleChange {
    int id = 0;
    std::string from_version;
    std::string to_version;
    std::string diff_summary;
    nlohmann::json changed_rules;
    std::string changed_by;
    std::string change_reason;
    std::string created_at;
};

class RuleRegistry {
public:
    static RuleRegistry& instance();

    void registerRule(const std::string& ruleName, 
                     const std::string& version,
                     const std::string& ruleType,
                     const nlohmann::json& ruleJson,
                     const std::string& description = "");

    std::optional<RuleRecord> getRule(const std::string& ruleName, const std::string& version);

    std::vector<RuleRecord> getRulesByType(const std::string& ruleType);

    std::vector<RuleRecord> getAllRules();

    void recordChange(const std::string& fromVersion,
                     const std::string& toVersion,
                     const nlohmann::json& changedRules,
                     const std::string& changedBy,
                     const std::string& reason);

    std::vector<RuleChange> getRuleChanges(const std::string& version = "");

    nlohmann::json getCurrentRulesSnapshot();

private:
    RuleRegistry() = default;
};
