#pragma once

#include "fault_injector.h"
#include <unordered_map>

class RecoveryScoreboard {
public:
    void recordFailure(ChaosFaultType type);
    void recordRecovery(ChaosFaultType type, double time_ms);

    double getResilienceScore() const;

    double getMTTR() const;

    double getRecoveryRate() const;

    void reset();

private:
    std::unordered_map<int, int> failure_count_;
    std::unordered_map<int, double> recovery_time_;
    int total_failures_ = 0;
    int successful_recoveries_ = 0;
    double total_recovery_time_ = 0.0;
};