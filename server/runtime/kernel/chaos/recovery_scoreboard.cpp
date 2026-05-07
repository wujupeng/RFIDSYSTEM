#include "recovery_scoreboard.h"

void RecoveryScoreboard::recordFailure(ChaosFaultType type) {
    int type_int = static_cast<int>(type);
    failure_count_[type_int]++;
    total_failures_++;
}

void RecoveryScoreboard::recordRecovery(ChaosFaultType type, double time_ms) {
    int type_int = static_cast<int>(type);
    recovery_time_[type_int] += time_ms;
    successful_recoveries_++;
    total_recovery_time_ += time_ms;
}

double RecoveryScoreboard::getResilienceScore() const {
    if (total_failures_ == 0) return 100.0;
    
    double rate = static_cast<double>(successful_recoveries_) / total_failures_;
    double mttr_score = std::max(0.0, 1.0 - getMTTR() / 500.0);
    
    return rate * 60 + mttr_score * 40;
}

double RecoveryScoreboard::getMTTR() const {
    if (successful_recoveries_ == 0) return 0.0;
    return total_recovery_time_ / successful_recoveries_;
}

double RecoveryScoreboard::getRecoveryRate() const {
    if (total_failures_ == 0) return 1.0;
    return static_cast<double>(successful_recoveries_) / total_failures_;
}

void RecoveryScoreboard::reset() {
    failure_count_.clear();
    recovery_time_.clear();
    total_failures_ = 0;
    successful_recoveries_ = 0;
    total_recovery_time_ = 0.0;
}