#pragma once

#include "../kernel_tick_context.h"

struct RecoveryResult {
    bool recovered;
    double recovery_time_ms;
    double post_recovery_trust;
    bool system_consistent;
};

class ResilienceVerifier {
public:
    static RecoveryResult verify(const KernelTickContext& before,
                                 const KernelTickContext& after);
};