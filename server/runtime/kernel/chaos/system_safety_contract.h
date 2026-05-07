#pragma once

struct SystemSafetyContract {
    // ❌ 禁止进入不可恢复状态
    static constexpr double MIN_TRUST_SCORE = 10.0;

    // ❌ GPU不能永久崩溃
    static constexpr int MAX_GPU_RESET = 5;

    // ❌ Frame不能永久丢失
    static constexpr double MAX_FRAME_DROP_RATE = 0.15;

    // ❌ AI不能无响应
    static constexpr int AI_TIMEOUT_MS = 200;

    // ✔ 必须满足：任何状态必须可回退
    static bool isRecoverableState(int trust_score) {
        return trust_score >= MIN_TRUST_SCORE;
    }
};