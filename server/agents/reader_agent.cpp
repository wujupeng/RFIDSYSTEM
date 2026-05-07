#include "reader_agent.h"

ReaderAgent::ReaderAgent(uint64_t reader_id) : reader_id_(reader_id) {}

std::string ReaderAgent::getName() const {
    return "ReaderAgent-" + std::to_string(reader_id_);
}

Observation ReaderAgent::observe() {
    Observation obs;
    obs.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    obs.type = "reader_status";
    
    obs.metrics.emplace_back("health_score", last_health_score_);
    obs.metrics.emplace_back("load", 0.3 + rand() % 50 / 100.0);
    obs.metrics.emplace_back("rssi", -60 - rand() % 30);
    
    return obs;
}

Decision ReaderAgent::think() {
    Decision decision;
    decision.decision_id = rand();
    decision.confidence = 0.85;
    
    if (last_health_score_ < 0.7) {
        decision.action = "inspect";
        decision.target_id = reader_id_;
        decision.reason = "Health score below threshold";
    } else {
        decision.action = "monitor";
        decision.target_id = reader_id_;
        decision.reason = "Normal operation";
    }
    
    return decision;
}

Action ReaderAgent::act(const Decision& decision) {
    Action action;
    action.action_id = decision.decision_id;
    action.type = decision.action;
    action.executed = true;
    action.result = "Action executed";
    
    return action;
}

void ReaderAgent::learn(const Feedback& feedback) {
    if (feedback.success) {
        confidence_ = std::min(1.0, confidence_ + 0.05);
    } else {
        confidence_ = std::max(0.1, confidence_ - 0.1);
    }
}