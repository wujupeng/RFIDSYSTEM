#pragma once

#include "kernel_abi.h"
#include <vector>
#include <string>

enum class KernelAction {
    NONE,
    CONTINUE,
    DOWNGRADE,
    RECOVER,
    EMERGENCY_STOP
};

struct KernelTickResult {
    KernelABI abi;
    KernelAction action;

    std::vector<std::string> violations;
};