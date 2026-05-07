#pragma once

#include <cstdint>

enum class ChaosFaultType {
    GPU_STALL,
    FRAME_DROP,
    AI_LATENCY_SPIKE,
    MEMORY_LEAK_SIM,
    NETWORK_PARTITION,
    DECISION_CORRUPTION,
    RANDOM_POLICY_FLIP
};

class FaultInjector {
public:
    static void inject(ChaosFaultType type, double intensity);

private:
    static void injectGPUStall(double intensity);
    static void injectFrameDrop(double intensity);
    static void injectAILatency(double intensity);
    static void injectMemoryLeakSim(double intensity);
    static void injectNetworkPartition(double intensity);
    static void injectDecisionCorruption(double intensity);
    static void injectRandomPolicyFlip(double intensity);
};