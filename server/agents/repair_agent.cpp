#include "repair_agent.h"

RepairAgent::RepairAgent() {}

std::string RepairAgent::getName() const {
    return "RepairAgent";
}

Observation RepairAgent::observe() {
    Observation obs;
    obs.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    obs.type = "system_health";
    
    active_failures_ = rand() % 3;
    obs.metrics.emplace_back("active_failures", active_failures_);
    obs.metrics.emplace_back("system_health", 0.7 + rand() % 20 / 100.0);
    obs.metrics.emplace_back("recovery_in_progress", active_failures_ > 0 ? 1.0 : 0.0);
    
    return obs;
}

Decision RepairAgent::think() {
    Decision decision;
    decision.decision_id = rand();
    
    if (active_failures_ > 0) {
        decision.action = "repair";
        decision.target_id = 1;
        decision.confidence = 0.85;
        decision.reason = "Active failures detected";
    } else {
        decision.action = "monitor";
        decision.target_id = 0;
        decision.confidence = 0.9;
        decision.reason = "System healthy";
    }
    
    return decision;
}

Action RepairAgent::act(const Decision& decision) {
    Action action;
    action.action_id = decision.decision_id;
    action.type = decision.action;
    action.executed = true;
    
    if (decision.action == "repair") {
        action.result = "Repaired " + std::to_string(active_failures_) + " failures";
        active_failures_ = 0;
    } else {
        action.result = "Monitoring system health";
    }
    
    return action;
}

void RepairAgent::learn(const Feedback& feedback) {
    if (feedback.success) {
        confidence_ = std::min(1.0, confidence_ + 0.05);
    } else {
        confidence_ = std::max(0.3, confidence_ - 0.1);
    }
}