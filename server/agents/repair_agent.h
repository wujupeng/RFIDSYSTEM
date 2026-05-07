#pragma once

#include "base_agent.h"

class RepairAgent : public BaseAgent {
public:
    RepairAgent();
    
    std::string getName() const override;
    
    Observation observe() override;
    
    Decision think() override;
    
    Action act(const Decision& decision) override;
    
    void learn(const Feedback& feedback) override;
    
private:
    uint64_t active_failures_ = 0;
};