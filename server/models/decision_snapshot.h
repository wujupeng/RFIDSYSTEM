#pragma once
#include <string>
#include <nlohmann/json.hpp>

struct DecisionSnapshot {
    int id = 0;
    int decision_id = 0;
    int asset_id = 0;

    double risk_missing = 0.0;
    double risk_inactivity = 0.0;
    double risk_abnormal = 0.0;

    int score = 0;

    std::string rule_version;
    std::string engine_version;
    
    nlohmann::json threshold_snapshot;
    nlohmann::json rule_snapshot_json;
};
