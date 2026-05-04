#include "rule_registry.h"
#include "db/db_pool.h"
#include <pqxx/pqxx>
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

RuleRegistry& RuleRegistry::instance() {
    static RuleRegistry instance;
    return instance;
}

void RuleRegistry::registerRule(const std::string& ruleName,
                               const std::string& version,
                               const std::string& ruleType,
                               const nlohmann::json& ruleJson,
                               const std::string& description) {
    auto conn = DBPool::instance().acquire();

    try {
        pqxx::work txn(*conn);

        std::string ruleJsonStr = ruleJson.dump();

        txn.exec_params(
            R"(
            INSERT INTO decision_rules
            (rule_name, rule_version, rule_type, rule_json, description)
            VALUES ($1, $2, $3, $4, $5)
            ON CONFLICT (rule_name, rule_version) DO UPDATE
            SET rule_json = EXCLUDED.rule_json,
                description = EXCLUDED.description,
                updated_at = NOW()
            )",
            ruleName,
            version,
            ruleType,
            ruleJsonStr,
            description
        );

        txn.commit();
    } catch (const std::exception& e) {
        spdlog::error("Register rule failed: {}", e.what());
    }

    DBPool::instance().release(conn);
}

std::optional<RuleRecord> RuleRegistry::getRule(const std::string& ruleName, const std::string& version) {
    auto conn = DBPool::instance().acquire();

    try {
        pqxx::work txn(*conn);

        auto res = txn.exec_params(
            R"(
            SELECT id, rule_name, rule_version, rule_type,
                   rule_json, description, enabled, created_at
            FROM decision_rules
            WHERE rule_name = $1 AND rule_version = $2
            )",
            ruleName,
            version
        );

        if (!res.empty()) {
            RuleRecord record;
            const auto& row = res[0];

            record.id = row["id"].as<int>();
            record.rule_name = row["rule_name"].c_str();
            record.rule_version = row["rule_version"].c_str();
            record.rule_type = row["rule_type"].c_str();
            record.rule_json = json::parse(row["rule_json"].c_str());
            record.description = row["description"].c_str();
            record.enabled = row["enabled"].as<bool>();
            record.created_at = row["created_at"].c_str();

            DBPool::instance().release(conn);
            return record;
        }

    } catch (const std::exception& e) {
        spdlog::error("Get rule failed: {}", e.what());
    }

    DBPool::instance().release(conn);
    return std::nullopt;
}

std::vector<RuleRecord> RuleRegistry::getRulesByType(const std::string& ruleType) {
    std::vector<RuleRecord> results;
    auto conn = DBPool::instance().acquire();

    try {
        pqxx::work txn(*conn);

        auto res = txn.exec_params(
            R"(
            SELECT id, rule_name, rule_version, rule_type,
                   rule_json, description, enabled, created_at
            FROM decision_rules
            WHERE rule_type = $1
            ORDER BY rule_name, rule_version
            )",
            ruleType
        );

        for (const auto& row : res) {
            RuleRecord record;
            record.id = row["id"].as<int>();
            record.rule_name = row["rule_name"].c_str();
            record.rule_version = row["rule_version"].c_str();
            record.rule_type = row["rule_type"].c_str();
            record.rule_json = json::parse(row["rule_json"].c_str());
            record.description = row["description"].c_str();
            record.enabled = row["enabled"].as<bool>();
            record.created_at = row["created_at"].c_str();
            results.push_back(record);
        }

    } catch (const std::exception& e) {
        spdlog::error("Get rules by type failed: {}", e.what());
    }

    DBPool::instance().release(conn);
    return results;
}

std::vector<RuleRecord> RuleRegistry::getAllRules() {
    std::vector<RuleRecord> results;
    auto conn = DBPool::instance().acquire();

    try {
        pqxx::work txn(*conn);

        auto res = txn.exec_params(
            R"(
            SELECT id, rule_name, rule_version, rule_type,
                   rule_json, description, enabled, created_at
            FROM decision_rules
            ORDER BY rule_name, rule_version
            )"
        );

        for (const auto& row : res) {
            RuleRecord record;
            record.id = row["id"].as<int>();
            record.rule_name = row["rule_name"].c_str();
            record.rule_version = row["rule_version"].c_str();
            record.rule_type = row["rule_type"].c_str();
            record.rule_json = json::parse(row["rule_json"].c_str());
            record.description = row["description"].c_str();
            record.enabled = row["enabled"].as<bool>();
            record.created_at = row["created_at"].c_str();
            results.push_back(record);
        }

    } catch (const std::exception& e) {
        spdlog::error("Get all rules failed: {}", e.what());
    }

    DBPool::instance().release(conn);
    return results;
}

void RuleRegistry::recordChange(const std::string& fromVersion,
                               const std::string& toVersion,
                               const nlohmann::json& changedRules,
                               const std::string& changedBy,
                               const std::string& reason) {
    auto conn = DBPool::instance().acquire();

    try {
        pqxx::work txn(*conn);

        std::string changedRulesStr = changedRules.dump();
        
        std::string diffSummary = "Rules changed from v" + fromVersion + " to v" + toVersion;
        if (!reason.empty()) {
            diffSummary += ": " + reason;
        }

        txn.exec_params(
            R"(
            INSERT INTO rule_changes
            (from_version, to_version, diff_summary, changed_rules, changed_by, change_reason)
            VALUES ($1, $2, $3, $4, $5, $6)
            )",
            fromVersion,
            toVersion,
            diffSummary,
            changedRulesStr,
            changedBy,
            reason
        );

        txn.commit();
    } catch (const std::exception& e) {
        spdlog::error("Record rule change failed: {}", e.what());
    }

    DBPool::instance().release(conn);
}

std::vector<RuleChange> RuleRegistry::getRuleChanges(const std::string& version) {
    std::vector<RuleChange> results;
    auto conn = DBPool::instance().acquire();

    try {
        pqxx::work txn(*conn);

        pqxx::result res;
        if (!version.empty()) {
            res = txn.exec_params(
                R"(
                SELECT id, from_version, to_version, diff_summary,
                       changed_rules, changed_by, change_reason, created_at
                FROM rule_changes
                WHERE to_version = $1 OR from_version = $1
                ORDER BY created_at DESC
                )",
                version
            );
        } else {
            res = txn.exec_params(
                R"(
                SELECT id, from_version, to_version, diff_summary,
                       changed_rules, changed_by, change_reason, created_at
                FROM rule_changes
                ORDER BY created_at DESC
                )"
            );
        }

        for (const auto& row : res) {
            RuleChange change;
            change.id = row["id"].as<int>();
            change.from_version = row["from_version"].c_str();
            change.to_version = row["to_version"].c_str();
            change.diff_summary = row["diff_summary"].c_str();
            
            if (!row["changed_rules"].is_null()) {
                change.changed_rules = json::parse(row["changed_rules"].c_str());
            }
            
            change.changed_by = row["changed_by"].c_str();
            change.change_reason = row["change_reason"].c_str();
            change.created_at = row["created_at"].c_str();
            results.push_back(change);
        }

    } catch (const std::exception& e) {
        spdlog::error("Get rule changes failed: {}", e.what());
    }

    DBPool::instance().release(conn);
    return results;
}

nlohmann::json RuleRegistry::getCurrentRulesSnapshot() {
    json snapshot;
    auto conn = DBPool::instance().acquire();

    try {
        pqxx::work txn(*conn);

        auto res = txn.exec_params(
            R"(
            SELECT rule_name, rule_version, rule_type, rule_json
            FROM decision_rules
            WHERE enabled = TRUE
            ORDER BY rule_type, rule_name
            )"
        );

        for (const auto& row : res) {
            std::string name = row["rule_name"].c_str();
            json ruleInfo;
            ruleInfo["version"] = row["rule_version"].c_str();
            ruleInfo["type"] = row["rule_type"].c_str();
            ruleInfo["content"] = json::parse(row["rule_json"].c_str());
            snapshot[name] = ruleInfo;
        }

    } catch (const std::exception& e) {
        spdlog::error("Get current rules snapshot failed: {}", e.what());
    }

    DBPool::instance().release(conn);
    return snapshot;
}
