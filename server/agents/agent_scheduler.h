#pragma once

#include "base_agent.h"
#include <vector>
#include <memory>
#include <mutex>

class AgentScheduler {
public:
    static AgentScheduler& instance();
    
    void addAgent(std::shared_ptr<BaseAgent> agent);
    
    void removeAgent(const std::string& name);
    
    void tick();
    
    size_t getAgentCount() const;
    
    void start();
    
    void stop();
    
    bool isRunning() const;
    
private:
    AgentScheduler();
    
    std::vector<std::shared_ptr<BaseAgent>> agents_;
    bool running_ = false;
    mutable std::mutex mutex_;
};