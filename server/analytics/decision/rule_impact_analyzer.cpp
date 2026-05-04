#include "rule_impact_analyzer.h"
#include "../../repository/decision_repository.h"
#include "../../repository/rule_registry.h"
#include "../../db/db_pool.h"
#include <pqxx/pqxx>
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>
#include <algorithm>

using json = nlohmann::json;

RuleImpactAnalyzer& RuleImpactAnalyzer::instance() {
    static RuleImpactAnalyzer instance;
    return instance;
}

std::vector<std::string> RuleImpactAnalyzer::findChangedRules(
    const std::string& from_version,
    const std::string& to_version) {
    
    std::vector<std::string> changedRules;
    auto changes = RuleRegistry::instance().getRuleChanges(to_version);
    
    for (const auto& change : changes) {
        if (change.from_version == from_version) {
            if (!change.changed_rules.empty()) {
                for (const auto& [ruleName, _] : change.changed_rules.items()) {
                    changedRules.push_back(ruleName);
                }
            }
        }
    }
    
    return changedRules;
}

double RuleImpactAnalyzer::calculateAccuracyDelta(
    const std::string& from_version,
    const std::string& to_version,
    const std::string& ruleName) {
    
    auto conn = DBPool::instance().acquire();
    double delta = 0.0;
    
    try {
        pqxx::work txn(*conn);
        
        auto res = txn.exec_params(
            R"(
            WITH version_stats AS (
                SELECT 
                    ds.rule_version,
                    COUNT(*) FILTER (WHERE d.executed = TRUE AND d.reason LIKE '%correct%') AS correct_executed,
                    COUNT(*) FILTER (WHERE d.executed = FALSE AND d.reason LIKE '%incorrect%') AS incorrect,
                    COUNT(*) AS total
                FROM decisions d
                JOIN decision_snapshots ds ON d.id = ds.decision_id
                WHERE ds.rule_version IN ($1, $2)
                GROUP BY ds.rule_version
            )
            SELECT 
                (SELECT correct_executed::FLOAT / NULLIF(total, 0) FROM version_stats WHERE rule_version = $2) -
                (SELECT correct_executed::FLOAT / NULLIF(total, 0) FROM version_stats WHERE rule_version = $1) AS delta
            )",
            from_version,
            to_version
        );
        
        if (!res.empty()) {
            delta = res[0][0].as<double>();
        }
        
    } catch (const std::exception& e) {
        spdlog::error("Calculate accuracy delta failed: {}", e.what());
    }
    
    DBPool::instance().release(conn);
    return delta;
}

RuleImpactReport RuleImpactAnalyzer::analyzeVersionDiff(
    const std::string& from_version,
    const std::string& to_version,
    int lookback_hours) {
    
    RuleImpactReport report;
    report.from_version = from_version;
    report.to_version = to_version;
    
    auto changedRules = findChangedRules(from_version, to_version);
    
    double overallAccuracyDelta = 0.0;
    double bestImpact = -1e9;
    double worstImpact = 1e9;
    
    for (const auto& ruleName : changedRules) {
        RuleImpactResult result;
        result.rule_name = ruleName;
        result.rule_version = to_version;
        
        result.impact_score = calculateAccuracyDelta(from_version, to_version, ruleName);
        overallAccuracyDelta += result.impact_score;
        
        auto conn = DBPool::instance().acquire();
        try {
            pqxx::work txn(*conn);
            
            auto res = txn.exec_params(
                R"(
                SELECT 
                    COUNT(*) FILTER (WHERE d.executed = TRUE) as true_positive,
                    COUNT(*) FILTER (WHERE d.ignored = TRUE) as false_positive,
                    COUNT(*) as total
                FROM decisions d
                JOIN decision_snapshots ds ON d.id = ds.decision_id
                WHERE ds.rule_version = $1
                  AND d.created_at > NOW() - ($2 * INTERVAL '1 hour')
                )",
                to_version,
                lookback_hours
            );
            
            if (!res.empty()) {
                int tp = res[0]["true_positive"].as<int>();
                int fp = res[0]["false_positive"].as<int>();
                int total = res[0]["total"].as<int>();
                
                result.precision_delta = total > 0 ? (double)tp / total : 0.0;
                result.false_positive_delta = total > 0 ? (double)fp / total : 0.0;
                result.sample_size = total;
            }
            
        } catch (const std::exception& e) {
            spdlog::error("Analyze rule {} failed: {}", ruleName, e.what());
        }
        DBPool::instance().release(conn);
        
        if (result.impact_score > bestImpact) {
            bestImpact = result.impact_score;
            report.best_performing_rule = ruleName;
        }
        if (result.impact_score < worstImpact) {
            worstImpact = result.impact_score;
            report.worst_performing_rule = ruleName;
        }
        
        report.impacts.push_back(result);
        storeImpactAnalysis(result);
    }
    
    report.overall_accuracy_delta = overallAccuracyDelta;
    
    return report;
}

DecisionAttributionResult RuleImpactAnalyzer::analyzeDecisionAttribution(int decisionId) {
    DecisionAttributionResult attribution;
    attribution.decision_id = decisionId;
    
    auto snapshotOpt = DecisionRepository::instance().getSnapshotByDecisionId(decisionId);
    if (!snapshotOpt) {
        return attribution;
    }
    
    auto& snapshot = snapshotOpt.value();
    attribution.snapshot_id = snapshot.id;
    
    double missing = snapshot.risk_missing;
    double inactivity = snapshot.risk_inactivity;
    double abnormal = snapshot.risk_abnormal;
    
    double total = missing + inactivity + abnormal;
    
    std::vector<std::pair<std::string, double>> contributions;
    
    if (total > 0) {
        contributions.emplace_back("missing_risk", missing / total);
        contributions.emplace_back("inactivity_risk", inactivity / total);
        contributions.emplace_back("abnormal_risk", abnormal / total);
    }
    
    std::sort(contributions.begin(), contributions.end(),
              [](const auto& a, const auto& b) { return a.second > b.second; });
    
    attribution.contributing_rules = contributions;
    
    if (!contributions.empty()) {
        attribution.primary_rule = contributions[0].first;
        attribution.primary_rule_contribution = contributions[0].second;
        attribution.attribution_confidence = contributions[0].second;
    }
    
    storeDecisionAttribution(attribution);
    
    return attribution;
}

std::vector<RuleImpactResult> RuleImpactAnalyzer::evaluateCurrentRules(int lookback_hours) {
    std::vector<RuleImpactResult> results;
    
    auto rules = RuleRegistry::instance().getAllRules();
    auto conn = DBPool::instance().acquire();
    
    try {
        pqxx::work txn(*conn);
        
        for (const auto& rule : rules) {
            RuleImpactResult result;
            result.rule_name = rule.rule_name;
            result.rule_version = rule.rule_version;
            
            auto res = txn.exec_params(
                R"(
                SELECT 
                    COUNT(*) FILTER (WHERE d.executed = TRUE) as executed,
                    COUNT(*) FILTER (WHERE d.ignored = TRUE) as ignored,
                    COUNT(*) as total,
                    AVG(ds.score) as avg_score
                FROM decisions d
                JOIN decision_snapshots ds ON d.id = ds.decision_id
                WHERE ds.rule_version = $1
                  AND d.created_at > NOW() - ($2 * INTERVAL '1 hour')
                )",
                rule.rule_version,
                lookback_hours
            );
            
            if (!res.empty()) {
                int executed = res[0]["executed"].as<int>();
                int ignored = res[0]["ignored"].as<int>();
                int total = res[0]["total"].as<int>();
                
                result.impact_score = total > 0 ? (double)executed / total : 0.0;
                result.sample_size = total;
                result.confidence_interval = total > 0 ? 1.96 / sqrt(total) : 0.0;
            }
            
            results.push_back(result);
            storeImpactAnalysis(result);
        }
        
    } catch (const std::exception& e) {
        spdlog::error("Evaluate current rules failed: {}", e.what());
    }
    
    DBPool::instance().release(conn);
    return results;
}

void RuleImpactAnalyzer::storeImpactAnalysis(const RuleImpactResult& result) {
    auto conn = DBPool::instance().acquire();
    
    try {
        pqxx::work txn(*conn);
        
        txn.exec_params(
            R"(
            INSERT INTO rule_impact_analysis
            (rule_version, rule_name, impact_score, precision_delta,
             recall_delta, f1_delta, adoption_rate_delta,
             false_positive_delta, false_negative_delta,
             sample_size, confidence_interval, analysis_summary)
            VALUES ($1, $2, $3, $4, $5, $6, $7, $8, $9, $10, $11, $12)
            )",
            result.rule_version,
            result.rule_name,
            result.impact_score,
            result.precision_delta,
            result.recall_delta,
            result.f1_delta,
            result.adoption_rate_delta,
            result.false_positive_delta,
            result.false_negative_delta,
            result.sample_size,
            result.confidence_interval,
            result.analysis_summary
        );
        
        txn.commit();
    } catch (const std::exception& e) {
        spdlog::error("Store impact analysis failed: {}", e.what());
    }
    
    DBPool::instance().release(conn);
}

void RuleImpactAnalyzer::storeDecisionAttribution(const DecisionAttributionResult& attribution) {
    auto conn = DBPool::instance().acquire();
    
    try {
        pqxx::work txn(*conn);
        
        json contributingRulesJson;
        for (const auto& [rule, contribution] : attribution.contributing_rules) {
            contributingRulesJson[rule] = contribution;
        }
        
        txn.exec_params(
            R"(
            INSERT INTO decision_attribution
            (decision_id, snapshot_id, primary_rule, primary_rule_contribution,
             contributing_rules, attribution_confidence)
            VALUES ($1, $2, $3, $4, $5, $6)
            )",
            attribution.decision_id,
            attribution.snapshot_id,
            attribution.primary_rule,
            attribution.primary_rule_contribution,
            contributingRulesJson.dump(),
            attribution.attribution_confidence
        );
        
        txn.commit();
    } catch (const std::exception& e) {
        spdlog::error("Store decision attribution failed: {}", e.what());
    }
    
    DBPool::instance().release(conn);
}
