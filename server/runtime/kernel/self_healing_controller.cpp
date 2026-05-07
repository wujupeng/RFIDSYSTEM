#include "self_healing_controller.h"

namespace runtime {

SelfHealingController::SelfHealingController()
    : recovery_engine_(&RuntimeRecoveryEngine::instance()) {
}

void SelfHealingController::update(int trust_score, const ConsistencyResult& consistency) {
    if (is_recovering_) {
        return;
    }

    triggerRecovery(trust_score);
}

bool SelfHealingController::isRecovering() const {
    return recovery_engine_->inRecovery();
}

RecoveryAction SelfHealingController::getLastAction() const {
    return last_action_;
}

void SelfHealingController::reset() {
    is_recovering_ = false;
    last_action_ = RecoveryAction::NONE;
}

void SelfHealingController::triggerRecovery(int trust_score) {
    if (trust_score > 80) return;

    if (trust_score < 10) {
        last_action_ = RecoveryAction::EMERGENCY_STOP;
    } else if (trust_score < 30) {
        last_action_ = RecoveryAction::RESTART_SUBSYSTEM;
    } else if (trust_score < 50) {
        last_action_ = RecoveryAction::SWITCH_TO_SAFE_POLICY;
    } else if (trust_score < 70) {
        last_action_ = RecoveryAction::REDUCE_FRAME_RATE;
    } else {
        return;
    }

    recovery_engine_->execute(last_action_);
}

} // namespace runtime