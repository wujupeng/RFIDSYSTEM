#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <map>

struct PolicyEvolution {
    uint64_t policy_id;
    uint64_t version;
    uint64_t timestamp;
    std::string change_reason;
    std::string changed_by;
    std::map<std::string, std::string> before_state;
    std::map<std::string, std::string> after_state;
};

struct RecoveryStatistics {
    uint64_t failure_type;
    uint64_t recovery_count;
    double avg_recovery_time_ms;
    double success_rate;
    std::string most_effective_method;
};

class EvolutionHistory {
public:
    void recordPolicyChange(const PolicyEvolution& evolution);
    
    std::vector<PolicyEvolution> getPolicyHistory(uint64_t policy_id);
    
    void recordRecoverySuccess(uint64_t failure_type, double recovery_time_ms);
    
    RecoveryStatistics getRecoveryStatistics(uint64_t failure_type);
    
    std::map<uint64_t, RecoveryStatistics> getAllRecoveryStats();
    
    void recordFactoryFailure(uint64_t factory_id);
    
    std::vector<uint64_t> getFrequentlyFailingFactories(int limit = 5);
    
private:
    std::vector<PolicyEvolution> policy_history_;
    std::map<uint64_t, std::vector<double>> recovery_times_;
    std::map<uint64_t, int> factory_failure_counts_;
};