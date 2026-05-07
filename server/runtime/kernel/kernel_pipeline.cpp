#include "kernel_pipeline.h"

bool KernelPipeline::checkConsistency(const KernelTickContext& ctx) {
    return true;
}

bool KernelPipeline::verifyProof(const KernelTickContext& ctx) {
    return true;
}

KernelAction KernelPipeline::heal(const KernelTickContext& ctx) {
    if (ctx.force_safe_policy) {
        return KernelAction::RECOVER;
    }
    
    return KernelAction::CONTINUE;
}