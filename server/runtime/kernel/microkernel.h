#pragma once

#include "kernel_tick_context.h"
#include "kernel_tick_result.h"

class MicroKernel {
public:
    static MicroKernel& instance();

    KernelTickResult tick(const KernelTickContext& ctx);

    int getTrustScore() const;

    void resetTrustScore();

private:
    MicroKernel() = default;

    int trust_score_ = 100;
};