#pragma once

#include "base_agent.h"

class PatrolAgent : public BaseAgent {
public:
    PatrolAgent();
    
    std::string getName() const override;
    
    Observation observe() override;
    
    Decision think() override;
    
    Action act(const Decision& decision) override;
    
    void learn(const Feedback& feedback) override;
    
private:
    uint64_t current_zone_ = 0;
};