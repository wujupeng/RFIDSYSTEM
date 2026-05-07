#pragma once

#include "../kernel_tick_context.h"

class ChaosKernel {
public:
    static ChaosKernel& instance();

    void tick(const KernelTickContext& ctx);

    void enableChaos(bool enable);

    bool isChaosEnabled() const;

private:
    ChaosKernel() = default;

    bool chaos_enabled_ = false;
};