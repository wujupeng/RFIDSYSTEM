#pragma once

#include "base_agent.h"

class ReaderAgent : public BaseAgent {
public:
    ReaderAgent(uint64_t reader_id);
    
    std::string getName() const override;
    
    Observation observe() override;
    
    Decision think() override;
    
    Action act(const Decision& decision) override;
    
    void learn(const Feedback& feedback) override;
    
private:
    uint64_t reader_id_;
    double last_health_score_ = 1.0;
    uint64_t last_check_timestamp_ = 0;
};