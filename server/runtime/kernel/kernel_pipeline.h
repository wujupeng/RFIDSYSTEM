#pragma once

#include "kernel_tick_context.h"
#include "kernel_tick_result.h"

class KernelPipeline {
public:
    // 1️⃣ Consistency Layer
    static bool checkConsistency(const KernelTickContext& ctx);

    // 2️⃣ Proof Layer
    static bool verifyProof(const KernelTickContext& ctx);

    // 3️⃣ Healing Layer
    static KernelAction heal(const KernelTickContext& ctx);
};