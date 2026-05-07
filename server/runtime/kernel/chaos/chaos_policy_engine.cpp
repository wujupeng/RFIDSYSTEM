#include "chaos_policy_engine.h"
#include "../spatial_random.h"

ChaosPolicy ChaosPolicyEngine::sample(double trust_score) {
    ChaosPolicy policy;
    
    if (trust_score > 80) {
        policy.probability = 0.3;
        policy.severity = 0.3;
    } else if (trust_score > 50) {
        policy.probability = 0.2;
        policy.severity = 0.2;
    } else {
        policy.probability = 0.1;
        policy.severity = 0.1;
    }
    
    policy.type = selectFaultType(trust_score);
    
    return policy;
}

ChaosFaultType ChaosPolicyEngine::selectFaultType(double trust_score) {
    auto& random = SpatialRandom::instance();
    double r = random.nextDouble();
    
    if (trust_score > 80) {
        if (r < 0.3) return ChaosFaultType::GPU_STALL;
        if (r < 0.5) return ChaosFaultType::AI_LATENCY_SPIKE;
        if (r < 0.7) return ChaosFaultType::FRAME_DROP;
        if (r < 0.85) return ChaosFaultType::DECISION_CORRUPTION;
        return ChaosFaultType::RANDOM_POLICY_FLIP;
    } else {
        if (r < 0.5) return ChaosFaultType::FRAME_DROP;
        return ChaosFaultType::AI_LATENCY_SPIKE;
    }
}