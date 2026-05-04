#include "action_generator.h"
#include "../../core/logger.h"
#include <sstream>
#include <algorithm>

namespace analytics {

ActionGenerator& ActionGenerator::instance() {
    static ActionGenerator instance;
    return instance;
}

ActionItem ActionGenerator::createUrgentAction(const Decision& decision) {
    ActionItem item;
    item.action = decision.action;
    item.reason = decision.reason;
    item.asset_name = decision.asset_name;
    item.priority = decision.priority;
    item.location = decision.suggested_location;
    item.is_automatic = false;
    return item;
}

ActionItem ActionGenerator::createNormalAction(const Decision& decision) {
    ActionItem item;
    item.action = decision.action;
    item.reason = decision.reason;
    item.asset_name = decision.asset_name;
    item.priority = decision.priority;
    item.location = decision.suggested_location;
    item.is_automatic = false;
    return item;
}

ActionItem ActionGenerator::createRoutineAction(const Decision& decision) {
    ActionItem item;
    item.action = decision.action;
    item.reason = decision.reason;
    item.asset_name = decision.asset_name;
    item.priority = decision.priority;
    item.location = decision.suggested_location;
    item.is_automatic = true;
    return item;
}

ActionReport ActionGenerator::generateReport(const DecisionResult& decisions) {
    ActionReport report;
    report.total_assets_analyzed = decisions.high_priority.size() +
                                   decisions.medium_priority.size() +
                                   decisions.low_priority.size();

    // 高优先级 - 紧急行动
    for (const auto& decision : decisions.high_priority) {
        if (decision.action_type != ActionType::NO_ACTION) {
            report.urgent_actions.push_back(createUrgentAction(decision));
        }
    }

    // 中优先级 - 正常行动
    for (const auto& decision : decisions.medium_priority) {
        if (decision.action_type != ActionType::NO_ACTION) {
            report.normal_actions.push_back(createNormalAction(decision));
        }
    }

    // 低优先级 - 例行行动
    for (const auto& decision : decisions.low_priority) {
        if (decision.action_type != ActionType::NO_ACTION) {
            report.routine_actions.push_back(createRoutineAction(decision));
        }
    }

    // 按优先级排序
    std::sort(report.urgent_actions.begin(), report.urgent_actions.end(),
        [](const ActionItem& a, const ActionItem& b) {
            return a.priority < b.priority;
        });

    spdlog::info("Action report generated: urgent={}, normal={}, routine={}",
        report.urgent_actions.size(), report.normal_actions.size(), report.routine_actions.size());

    return report;
}

std::string ActionGenerator::formatReport(const ActionReport& report) {
    std::ostringstream oss;

    oss << "========================================\n";
    oss << "        资产决策行动报告\n";
    oss << "========================================\n";
    oss << "分析资产数: " << report.total_assets_analyzed << "\n\n";

    if (!report.urgent_actions.empty()) {
        oss << "【紧急行动】 - 立即处理\n";
        for (size_t i = 0; i < report.urgent_actions.size(); ++i) {
            const auto& action = report.urgent_actions[i];
            oss << (i + 1) << ". " << action.asset_name;
            oss << " → " << action.action;
            oss << " (优先级" << action.priority << ")\n";
            oss << "   原因: " << action.reason << "\n";
            if (!action.location.empty()) {
                oss << "   位置: " << action.location << "\n";
            }
            oss << "\n";
        }
    }

    if (!report.normal_actions.empty()) {
        oss << "【正常行动】 - 今日处理\n";
        for (size_t i = 0; i < report.normal_actions.size(); ++i) {
            const auto& action = report.normal_actions[i];
            oss << (i + 1) << ". " << action.asset_name;
            oss << " → " << action.action;
            oss << " (优先级" << action.priority << ")\n";
            oss << "   原因: " << action.reason << "\n\n";
        }
    }

    if (!report.routine_actions.empty()) {
        oss << "【例行行动】 - 本周处理\n";
        for (size_t i = 0; i < report.routine_actions.size(); ++i) {
            const auto& action = report.routine_actions[i];
            oss << (i + 1) << ". " << action.asset_name;
            oss << " → " << action.action;
            oss << " (优先级" << action.priority << ")\n";
            oss << "   原因: " << action.reason << "\n\n";
        }
    }

    if (report.urgent_actions.empty() &&
        report.normal_actions.empty() &&
        report.routine_actions.empty()) {
        oss << "所有资产状态良好，无需特殊操作。\n";
    }

    oss << "========================================\n";

    return oss.str();
}

} // namespace analytics