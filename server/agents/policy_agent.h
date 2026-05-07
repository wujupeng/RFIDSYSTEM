#pragma once

#include "base_agent.h"

class PolicyAgent : public BaseAgent {
public:
    PolicyAgent();
    
    std::string getName() const override;
    
    Observation observe() override;
    
    Decision think() override;
    
    Action act(const Decision& decision) override;
    
    void learn(const Feedback& feedback) override;
    
private:
    std::string current_policy_ = "default";
};