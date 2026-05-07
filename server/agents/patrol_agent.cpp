#include "patrol_agent.h"

PatrolAgent::PatrolAgent() {}

std::string PatrolAgent::getName() const {
    return "PatrolAgent";
}

Observation PatrolAgent::observe() {
    Observation obs;
    obs.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    obs.type = "zone_status";
    
    obs.metrics.emplace_back("current_zone", current_zone_);
    obs.metrics.emplace_back("zone_activity", 0.2 + rand() % 60 / 100.0);
    obs.metrics.emplace_back("anomaly_detected", rand() % 10 < 2 ? 1.0 : 0.0);
    
    return obs;
}

Decision PatrolAgent::think() {
    Decision decision;
    decision.decision_id = rand();
    decision.confidence = 0.7;
    decision.action = "patrol";
    decision.target_id = (current_zone_ + 1) % 10;
    decision.reason = "Regular patrol cycle";
    
    return decision;
}

Action PatrolAgent::act(const Decision& decision) {
    Action action;
    action.action_id = decision.decision_id;
    action.type = decision.action;
    action.executed = true;
    action.result = "Patrolled zone " + std::to_string(decision.target_id);
    
    current_zone_ = decision.target_id;
    
    return action;
}

void PatrolAgent::learn(const Feedback& feedback) {
    if (feedback.success) {
        confidence_ = std::min(1.0, confidence_ + 0.02);
    }
}