#pragma once

#include <cstdint>
#include <vector>

enum class KernelMode {
    NORMAL,
    DEGRADED,
    SAFE,
    INSPECT,
    EMERGENCY
};

struct KernelABI {
    uint64_t frame_id;
    uint64_t timestamp;

    uint64_t frame_hash;
    uint64_t ai_hash;
    uint64_t gpu_hash;

    double trust_score;
    KernelMode mode;

    bool deterministic_ok;
    bool causal_ok;
    bool recovery_ok;
};