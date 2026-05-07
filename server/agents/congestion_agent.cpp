#include "congestion_agent.h"

CongestionAgent::CongestionAgent() {}

std::string CongestionAgent::getName() const {
    return "CongestionAgent";
}

Observation CongestionAgent::observe() {
    Observation obs;
    obs.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    obs.type = "congestion";
    
    congestion_level_ = 0.2 + rand() % 60 / 100.0;
    obs.metrics.emplace_back("congestion_level", congestion_level_);
    obs.metrics.emplace_back("queue_length", 10 + rand() % 50);
    obs.metrics.emplace_back("flow_rate", 100 - rand() % 40);
    
    return obs;
}

Decision CongestionAgent::think() {
    Decision decision;
    decision.decision_id = rand();
    
    if (congestion_level_ > 0.7) {
        decision.action = "redirect";
        decision.target_id = 1;
        decision.confidence = 0.9;
        decision.reason = "High congestion detected";
    } else if (congestion_level_ > 0.5) {
        decision.action = "monitor";
        decision.target_id = 0;
        decision.confidence = 0.8;
        decision.reason = "Moderate congestion";
    } else {
        decision.action = "idle";
        decision.target_id = 0;
        decision.confidence = 0.95;
        decision.reason = "Normal flow";
    }
    
    return decision;
}

Action CongestionAgent::act(const Decision& decision) {
    Action action;
    action.action_id = decision.decision_id;
    action.type = decision.action;
    action.executed = true;
    action.result = "Congestion action: " + decision.action;
    
    return action;
}

void CongestionAgent::learn(const Feedback& feedback) {
    if (feedback.success && feedback.reward > 0.5) {
        confidence_ = std::min(1.0, confidence_ + 0.03);
    }
}