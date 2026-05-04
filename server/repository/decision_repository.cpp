#include "decision_repository.h"
#include "db/db_pool.h"
#include <pqxx/pqxx>
#include <spdlog/spdlog.h>
#include <string>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

DecisionRepository& DecisionRepository::instance() {
    static DecisionRepository instance;
    return instance;
}

void DecisionRepository::insert(const DecisionRecord& r)
{
    auto conn = DBPool::instance().acquire();

    try {
        pqxx::work txn(*conn);

        txn.exec_params(
            R"(
            INSERT INTO decisions
            (asset_id, asset_name, location, action, risk_level, reason, created_at)
            VALUES ($1, $2, $3, $4, $5, $6, NOW())
            )",
            r.asset_id,
            r.asset_name,
            r.location,
            r.action,
            r.risk_level,
            r.reason
        );

        txn.commit();
    } catch (const std::exception& e) {
        spdlog::error("Insert decision failed: {}", e.what());
    }

    DBPool::instance().release(conn);
}

int DecisionRepository::insertWithId(const DecisionRecord& r)
{
    auto conn = DBPool::instance().acquire();
    int newId = 0;

    try {
        pqxx::work txn(*conn);

        auto res = txn.exec_params(
            R"(
            INSERT INTO decisions
            (asset_id, asset_name, location, action, risk_level, reason, created_at)
            VALUES ($1, $2, $3, $4, $5, $6, NOW())
            RETURNING id
            )",
            r.asset_id,
            r.asset_name,
            r.location,
            r.action,
            r.risk_level,
            r.reason
        );

        if (!res.empty()) {
            newId = res[0][0].as<int>();
        }

        txn.commit();
    } catch (const std::exception& e) {
        spdlog::error("InsertWithId decision failed: {}", e.what());
    }

    DBPool::instance().release(conn);
    return newId;
}

std::vector<DecisionRecord> DecisionRepository::getRecent(int limit)
{
    std::vector<DecisionRecord> results;

    auto conn = DBPool::instance().acquire();

    try {
        pqxx::work txn(*conn);

        auto res = txn.exec_params(
            R"(
            SELECT id, asset_id, asset_name, location,
                   action, risk_level, reason,
                   executed, ignored, operator_name,
                   created_at
            FROM decisions
            ORDER BY created_at DESC
            LIMIT $1
            )",
            limit
        );

        for (const auto& row : res) {
            DecisionRecord r;

            r.id = row["id"].as<int>();
            r.asset_id = row["asset_id"].as<int>();
            r.asset_name = row["asset_name"].c_str();
            r.location = row["location"].c_str();
            r.action = row["action"].c_str();
            r.risk_level = row["risk_level"].c_str();
            r.reason = row["reason"].c_str();

            r.executed = row["executed"].as<bool>();
            r.ignored = row["ignored"].as<bool>();

            if (!row["operator_name"].is_null())
                r.operator_name = row["operator_name"].c_str();

            r.created_at = row["created_at"].c_str();

            results.push_back(r);
        }

    } catch (const std::exception& e) {
        spdlog::error("GetRecent failed: {}", e.what());
    }

    DBPool::instance().release(conn);
    return results;
}

void DecisionRepository::markExecuted(
    int assetId,
    bool executed,
    bool ignored,
    const std::string& operatorName)
{
    auto conn = DBPool::instance().acquire();

    try {
        pqxx::work txn(*conn);

        txn.exec_params(
            R"(
            UPDATE decisions
            SET executed = $1,
                ignored = $2,
                operator_name = $3,
                updated_at = NOW()
            WHERE asset_id = $4
              AND created_at = (
                  SELECT MAX(created_at)
                  FROM decisions
                  WHERE asset_id = $4
              )
            )",
            executed,
            ignored,
            operatorName,
            assetId
        );

        txn.commit();
    } catch (const std::exception& e) {
        spdlog::error("markExecuted failed: {}", e.what());
    }

    DBPool::instance().release(conn);
}

double DecisionRepository::getAdoptionRate(int hours)
{
    auto conn = DBPool::instance().acquire();
    double rate = 0.0;

    try {
        pqxx::work txn(*conn);

        auto res = txn.exec_params(
            R"(
            SELECT
                COUNT(*) FILTER (WHERE executed = TRUE) AS executed_count,
                COUNT(*) AS total_count
            FROM decisions
            WHERE created_at > NOW() - ($1 * INTERVAL '1 hour')
            )",
            hours
        );

        int executed = res[0]["executed_count"].as<int>();
        int total = res[0]["total_count"].as<int>();

        if (total > 0)
            rate = static_cast<double>(executed) / total;

    } catch (const std::exception& e) {
        spdlog::error("getAdoptionRate failed: {}", e.what());
    }

    DBPool::instance().release(conn);
    return rate;
}

void DecisionRepository::insertSnapshot(const DecisionSnapshot& snapshot)
{
    auto conn = DBPool::instance().acquire();

    try {
        pqxx::work txn(*conn);

        std::string thresholdJson = snapshot.threshold_snapshot.dump();
        std::string ruleSnapshotJson = snapshot.rule_snapshot_json.dump();

        txn.exec_params(
            R"(
            INSERT INTO decision_snapshots
            (decision_id, asset_id, risk_missing, risk_inactivity,
             risk_abnormal, score, rule_version, threshold_snapshot, 
             engine_version, rule_snapshot_json)
            VALUES ($1, $2, $3, $4, $5, $6, $7, $8, $9, $10)
            )",
            snapshot.decision_id,
            snapshot.asset_id,
            snapshot.risk_missing,
            snapshot.risk_inactivity,
            snapshot.risk_abnormal,
            snapshot.score,
            snapshot.rule_version,
            thresholdJson,
            snapshot.engine_version,
            ruleSnapshotJson
        );

        txn.commit();
    } catch (const std::exception& e) {
        spdlog::error("Insert snapshot failed: {}", e.what());
    }

    DBPool::instance().release(conn);
}

std::optional<DecisionSnapshot> DecisionRepository::getSnapshotByDecisionId(int decisionId)
{
    auto conn = DBPool::instance().acquire();

    try {
        pqxx::work txn(*conn);

        auto res = txn.exec_params(
            R"(
            SELECT id, decision_id, asset_id, risk_missing,
                   risk_inactivity, risk_abnormal, score,
                   rule_version, threshold_snapshot, engine_version,
                   rule_snapshot_json
            FROM decision_snapshots
            WHERE decision_id = $1
            )",
            decisionId
        );

        if (!res.empty()) {
            DecisionSnapshot snapshot;
            const auto& row = res[0];

            snapshot.id = row["id"].as<int>();
            snapshot.decision_id = row["decision_id"].as<int>();
            snapshot.asset_id = row["asset_id"].as<int>();
            snapshot.risk_missing = row["risk_missing"].as<double>();
            snapshot.risk_inactivity = row["risk_inactivity"].as<double>();
            snapshot.risk_abnormal = row["risk_abnormal"].as<double>();
            snapshot.score = row["score"].as<int>();
            snapshot.rule_version = row["rule_version"].c_str();
            snapshot.engine_version = row["engine_version"].c_str();

            if (!row["threshold_snapshot"].is_null()) {
                snapshot.threshold_snapshot = json::parse(row["threshold_snapshot"].c_str());
            }

            if (!row["rule_snapshot_json"].is_null()) {
                snapshot.rule_snapshot_json = json::parse(row["rule_snapshot_json"].c_str());
            }

            DBPool::instance().release(conn);
            return snapshot;
        }

    } catch (const std::exception& e) {
        spdlog::error("GetSnapshot failed: {}", e.what());
    }

    DBPool::instance().release(conn);
    return std::nullopt;
}
