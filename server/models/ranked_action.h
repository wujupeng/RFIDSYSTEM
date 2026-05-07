#pragma once
#include <string>
#include <vector>

namespace v33 {

enum class ActionType {
    NO_ACTION,
    INSPECT,
    ALERT,
    REALLOCATE
};

struct RankedAction {
    ActionType action;
    double score;
    double uncertainty;
    double confidence;
};

enum class DecisionMode {
    NORMAL,
    SHADOW,
    VALIDATION
};

struct DecisionContext {
    int asset_id;
    std::string asset_name;
    std::string current_location;
    bool in_illegal_location;
    double missing_risk;
    double inactivity_risk;
    double abnormal_risk;
    int move_count_24h;
    int missing_hours;
    double daily_avg_scans;
};

}
