#pragma once

#include <string>
#include <vector>
#include <memory>

struct Observation {
    uint64_t timestamp;
    std::string type;
    std::vector<std::pair<std::string, double>> metrics;
};

struct Decision {
    uint64_t decision_id;
    std::string action;
    uint64_t target_id;
    double confidence;
    std::string reason;
};

struct Action {
    uint64_t action_id;
    std::string type;
    bool executed;
    std::string result;
};

struct Feedback {
    bool success;
    double reward;
    std::string evaluation;
};

class BaseAgent {
public:
    virtual ~BaseAgent() = default;
    
    virtual std::string getName() const = 0;
    virtual Observation observe() = 0;
    virtual Decision think() = 0;
    virtual Action act(const Decision& decision) = 0;
    virtual void learn(const Feedback& feedback) = 0;
    
    virtual bool isActive() const { return active_; }
    virtual void setActive(bool active) { active_ = active; }
    
    virtual double getConfidence() const { return confidence_; }
    
protected:
    bool active_ = true;
    double confidence_ = 0.5;
};