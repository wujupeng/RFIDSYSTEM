#pragma once

#include "base_agent.h"

class CongestionAgent : public BaseAgent {
public:
    CongestionAgent();
    
    std::string getName() const override;
    
    Observation observe() override;
    
    Decision think() override;
    
    Action act(const Decision& decision) override;
    
    void learn(const Feedback& feedback) override;
    
private:
    double congestion_level_ = 0.0;
};