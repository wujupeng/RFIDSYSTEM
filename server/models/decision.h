#pragma once
#include <string>

struct DecisionRecord {
    int id = 0;

    int asset_id = 0;
    std::string asset_name;
    std::string location;

    std::string action;
    std::string risk_level;
    std::string reason;

    bool executed = false;
    bool ignored = false;
    std::string operator_name;

    std::string created_at;
};
