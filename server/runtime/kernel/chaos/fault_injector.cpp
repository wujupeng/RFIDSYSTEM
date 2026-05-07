#include "fault_injector.h"
#include <chrono>
#include <thread>

void FaultInjector::inject(ChaosFaultType type, double intensity) {
    switch (type) {
        case ChaosFaultType::GPU_STALL:
            injectGPUStall(intensity);
            break;
        case ChaosFaultType::FRAME_DROP:
            injectFrameDrop(intensity);
            break;
        case ChaosFaultType::AI_LATENCY_SPIKE:
            injectAILatency(intensity);
            break;
        case ChaosFaultType::MEMORY_LEAK_SIM:
            injectMemoryLeakSim(intensity);
            break;
        case ChaosFaultType::NETWORK_PARTITION:
            injectNetworkPartition(intensity);
            break;
        case ChaosFaultType::DECISION_CORRUPTION:
            injectDecisionCorruption(intensity);
            break;
        case ChaosFaultType::RANDOM_POLICY_FLIP:
            injectRandomPolicyFlip(intensity);
            break;
    }
}

void FaultInjector::injectGPUStall(double intensity) {
    int stall_ms = static_cast<int>(100 + intensity * 400);
    std::this_thread::sleep_for(std::chrono::milliseconds(stall_ms));
}

void FaultInjector::injectFrameDrop(double intensity) {
}

void FaultInjector::injectAILatency(double intensity) {
    int delay_ms = static_cast<int>(50 + intensity * 300);
    std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
}

void FaultInjector::injectMemoryLeakSim(double intensity) {
}

void FaultInjector::injectNetworkPartition(double intensity) {
}

void FaultInjector::injectDecisionCorruption(double intensity) {
}

void FaultInjector::injectRandomPolicyFlip(double intensity) {
}