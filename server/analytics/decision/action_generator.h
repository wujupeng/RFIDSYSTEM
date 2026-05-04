#pragma once
#include <string>
#include <vector>
#include "decision_engine.h"

namespace analytics {

struct ActionItem {
    std::string action;
    std::string reason;
    std::string asset_name;
    int priority;
    std::string location;
    bool is_automatic;
};

struct ActionReport {
    std::vector<ActionItem> urgent_actions;
    std::vector<ActionItem> normal_actions;
    std::vector<ActionItem> routine_actions;
    int total_assets_analyzed;
};

class ActionGenerator {
public:
    static ActionGenerator& instance();

    ActionReport generateReport(const DecisionResult& decisions);

    ActionItem createUrgentAction(const Decision& decision);
    ActionItem createNormalAction(const Decision& decision);
    ActionItem createRoutineAction(const Decision& decision);

    std::string formatReport(const ActionReport& report);

private:
    ActionGenerator() = default;
    ActionGenerator(const ActionGenerator&) = delete;
    ActionGenerator& operator=(const ActionGenerator&) = delete;
};

} // namespace analytics