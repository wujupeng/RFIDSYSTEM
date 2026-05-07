#include "system_consistency_checker.h"
#include <cmath>

namespace runtime {

SystemConsistencyChecker::SystemConsistencyChecker() {}

ConsistencyResult SystemConsistencyChecker::verify(const ConsistencyInput& input) {
    std::lock_guard<std::mutex> lock(mutex_);

    ConsistencyResult result;

    // =========================
    // 1. Frame一致性
    // =========================
    result.frame_consistent =
        verifyFrame(input.frame_hash, input.replay_frame_hash);

    if (!result.frame_consistent) {
        result.violations.push_back("FRAME_MISMATCH");
    }

    // =========================
    // 2. AI一致性
    // =========================
    result.ai_consistent =
        verifyAI(input.ai_hash, input.replay_ai_hash);

    if (!result.ai_consistent) {
        result.violations.push_back("AI_MISMATCH");
    }

    // =========================
    // 3. GPU一致性
    // =========================
    result.gpu_consistent =
        verifyGPU(input.gpu_hash, input.render_hash);

    if (!result.gpu_consistent) {
        result.violations.push_back("GPU_MISMATCH");
    }

    // =========================
    // 4. Confidence计算
    // =========================
    result.confidence = calculateConfidence(
        result.frame_consistent,
        result.ai_consistent,
        result.gpu_consistent
    );

    // =========================
    // 5. failure streak更新
    // =========================
    if (!result.frame_consistent ||
        !result.ai_consistent ||
        !result.gpu_consistent) {
        failure_streak_++;
    } else {
        failure_streak_ = 0;
    }

    return result;
}

// =========================
// Frame一致性
// =========================
bool SystemConsistencyChecker::verifyFrame(uint64_t realtime_hash,
                                           uint64_t replay_hash) {
    last_frame_ok_ = (realtime_hash == replay_hash);
    return last_frame_ok_;
}

// =========================
// AI一致性
// =========================
bool SystemConsistencyChecker::verifyAI(uint64_t ai_hash,
                                        uint64_t replay_ai_hash) {
    last_ai_ok_ = (ai_hash == replay_ai_hash);
    return last_ai_ok_;
}

// =========================
// GPU一致性
// =========================
bool SystemConsistencyChecker::verifyGPU(uint64_t gpu_hash,
                                         uint64_t render_hash) {
    last_gpu_ok_ = (gpu_hash == render_hash);
    return last_gpu_ok_;
}

// =========================
// 系统风险判断
// =========================
bool SystemConsistencyChecker::isSystemAtRisk() const {
    return failure_streak_ >= 3;
}

// =========================
// failure streak
// =========================
int SystemConsistencyChecker::getFailureStreak() const {
    return failure_streak_;
}

// =========================
// reset（Recovery用）
// =========================
void SystemConsistencyChecker::reset() {
    std::lock_guard<std::mutex> lock(mutex_);
    failure_streak_ = 0;
    last_frame_ok_ = true;
    last_ai_ok_ = true;
    last_gpu_ok_ = true;
}

// =========================
// confidence计算
// =========================
double SystemConsistencyChecker::calculateConfidence(bool f, bool a, bool g) {
    int score = 0;
    if (f) score++;
    if (a) score++;
    if (g) score++;

    return static_cast<double>(score) / 3.0;
}

} // namespace runtime