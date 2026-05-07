#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <mutex>

namespace runtime {

/**
 * 三层一致性验证结果
 */
struct ConsistencyResult {
    bool frame_consistent = false;
    bool ai_consistent = false;
    bool gpu_consistent = false;

    double confidence = 0.0;

    std::vector<std::string> violations;
};

/**
 * 三类hash输入（系统三大核心）
 */
struct ConsistencyInput {
    uint64_t frame_hash = 0;
    uint64_t replay_frame_hash = 0;

    uint64_t ai_hash = 0;
    uint64_t replay_ai_hash = 0;

    uint64_t gpu_hash = 0;
    uint64_t render_hash = 0;

    uint64_t frame_id = 0;
};

/**
 * 系统一致性验证器（核心）
 *
 * 职责：
 * 1. Frame一致性（Realtime vs Replay）
 * 2. AI一致性（Executor vs Replay AI）
 * 3. GPU一致性（CPU Hash vs Render Output）
 *
 * ⚠️ 不允许参与任何业务逻辑，只做验证
 */
class SystemConsistencyChecker {
public:
    SystemConsistencyChecker();

    /**
     * 主入口：完整系统一致性验证
     */
    ConsistencyResult verify(const ConsistencyInput& input);

    /**
     * 单独验证 Frame
     */
    bool verifyFrame(uint64_t realtime_hash,
                     uint64_t replay_hash);

    /**
     * 单独验证 AI
     */
    bool verifyAI(uint64_t ai_hash,
                  uint64_t replay_ai_hash);

    /**
     * 单独验证 GPU
     */
    bool verifyGPU(uint64_t gpu_hash,
                   uint64_t render_hash);

    /**
     * 是否进入危险状态
     */
    bool isSystemAtRisk() const;

    /**
     * 连续失败计数
     */
    int getFailureStreak() const;

    /**
     * 重置状态（Recovery调用）
     */
    void reset();

private:
    bool last_frame_ok_ = true;
    bool last_ai_ok_ = true;
    bool last_gpu_ok_ = true;

    int failure_streak_ = 0;

    mutable std::mutex mutex_;

    double calculateConfidence(bool f, bool a, bool g);
};

} // namespace runtime