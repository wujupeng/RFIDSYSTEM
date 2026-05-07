#include "evolution_history.h"
#include <algorithm>
#include <numeric>

void EvolutionHistory::recordPolicyChange(const PolicyEvolution& evolution) {
    policy_history_.push_back(evolution);
}

std::vector<PolicyEvolution> EvolutionHistory::getPolicyHistory(uint64_t policy_id) {
    std::vector<PolicyEvolution> result;
    for (const auto& ev : policy_history_) {
        if (ev.policy_id == policy_id) {
            result.push_back(ev);
        }
    }
    std::sort(result.begin(), result.end(), 
        [](const PolicyEvolution& a, const PolicyEvolution& b) { return a.timestamp < b.timestamp; });
    return result;
}

void EvolutionHistory::recordRecoverySuccess(uint64_t failure_type, double recovery_time_ms) {
    recovery_times_[failure_type].push_back(recovery_time_ms);
}

RecoveryStatistics EvolutionHistory::getRecoveryStatistics(uint64_t failure_type) {
    RecoveryStatistics stats;
    stats.failure_type = failure_type;
    
    auto it = recovery_times_.find(failure_type);
    if (it != recovery_times_.end()) {
        const auto& times = it->second;
        stats.recovery_count = times.size();
        stats.avg_recovery_time_ms = std::accumulate(times.begin(), times.end(), 0.0) / times.size();
        stats.success_rate = 1.0;
    }
    
    return stats;
}

std::map<uint64_t, RecoveryStatistics> EvolutionHistory::getAllRecoveryStats() {
    std::map<uint64_t, RecoveryStatistics> result;
    for (const auto& pair : recovery_times_) {
        result[pair.first] = getRecoveryStatistics(pair.first);
    }
    return result;
}

void EvolutionHistory::recordFactoryFailure(uint64_t factory_id) {
    factory_failure_counts_[factory_id]++;
}

std::vector<uint64_t> EvolutionHistory::getFrequentlyFailingFactories(int limit) {
    std::vector<std::pair<uint64_t, int>> factories(factory_failure_counts_.begin(), factory_failure_counts_.end());
    
    std::sort(factories.begin(), factories.end(),
        [](const std::pair<uint64_t, int>& a, const std::pair<uint64_t, int>& b) {
            return a.second > b.second;
        });
    
    std::vector<uint64_t> result;
    for (int i = 0; i < limit && i < factories.size(); ++i) {
        result.push_back(factories[i].first);
    }
    
    return result;
}