#pragma once

#include "kernel_abi.h"
#include "kernel_tick_result.h"

class KernelPolicyRouter {
public:
    static KernelAction route(KernelMode mode, int trust_score);

    static bool shouldReduceFrameRate(KernelMode mode);
    static bool shouldDisableAI(KernelMode mode);
    static bool shouldSwitchToSafePolicy(KernelMode mode);
};