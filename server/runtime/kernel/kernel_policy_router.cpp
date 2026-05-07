#include "kernel_policy_router.h"

KernelAction KernelPolicyRouter::route(KernelMode mode, int trust_score) {
    switch (mode) {
        case KernelMode::NORMAL:
            return KernelAction::CONTINUE;
        case KernelMode::DEGRADED:
            return KernelAction::DOWNGRADE;
        case KernelMode::SAFE:
            return KernelAction::RECOVER;
        case KernelMode::INSPECT:
            return KernelAction::RECOVER;
        case KernelMode::EMERGENCY:
            return KernelAction::EMERGENCY_STOP;
        default:
            return KernelAction::NONE;
    }
}

bool KernelPolicyRouter::shouldReduceFrameRate(KernelMode mode) {
    return mode == KernelMode::DEGRADED || 
           mode == KernelMode::SAFE;
}

bool KernelPolicyRouter::shouldDisableAI(KernelMode mode) {
    return mode == KernelMode::INSPECT || 
           mode == KernelMode::EMERGENCY;
}

bool KernelPolicyRouter::shouldSwitchToSafePolicy(KernelMode mode) {
    return mode == KernelMode::SAFE || 
           mode == KernelMode::INSPECT;
}