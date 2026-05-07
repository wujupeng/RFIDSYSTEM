#pragma once

#include "fault_injector.h"

struct ChaosPolicy {
    double probability;
    double severity;
    ChaosFaultType type;
};

class ChaosPolicyEngine {
public:
    static ChaosPolicy sample(double trust_score);

private:
    static ChaosFaultType selectFaultType(double trust_score);
};