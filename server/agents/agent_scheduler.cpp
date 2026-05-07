#include "agent_scheduler.h"
#include <algorithm>

AgentScheduler::AgentScheduler() {}

AgentScheduler& AgentScheduler::instance() {
    static AgentScheduler instance;
    return instance;
}

void AgentScheduler::addAgent(std::shared_ptr<BaseAgent> agent) {
    std::lock_guard<std::mutex> lock(mutex_);
    agents_.push_back(agent);
}

void AgentScheduler::removeAgent(const std::string& name) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = std::remove_if(agents_.begin(), agents_.end(),
        [name](const std::shared_ptr<BaseAgent>& a) { return a->getName() == name; });
    agents_.erase(it, agents_.end());
}

void AgentScheduler::tick() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    for (auto& agent : agents_) {
        if (!agent->isActive()) continue;
        
        Observation obs = agent->observe();
        Decision decision = agent->think();
        Action action = agent->act(decision);
        
        Feedback feedback;
        feedback.success = action.executed;
        feedback.reward = action.executed ? 0.8 : 0.2;
        feedback.evaluation = action.result;
        
        agent->learn(feedback);
    }
}

size_t AgentScheduler::getAgentCount() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return agents_.size();
}

void AgentScheduler::start() {
    running_ = true;
}

void AgentScheduler::stop() {
    running_ = false;
}

bool AgentScheduler::isRunning() const {
    return running_;
}