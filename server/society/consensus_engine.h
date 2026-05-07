#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <map>

enum class ConsensusResult {
    CONSENSUS_REACHED,
    NO_CONSENSUS,
    DEADLOCK,
    ABSTAINED
};

struct ConsensusProposal {
    uint64_t proposal_id;
    std::string action;
    uint64_t target_id;
    std::string description;
};

struct ConsensusVote {
    std::string agent;
    std::string preferred_action;
    double confidence;
    std::string reason;
};

class ConsensusEngine {
public:
    static ConsensusEngine& instance();
    
    ConsensusResult reachConsensus(const ConsensusProposal& proposal,
                                  const std::vector<ConsensusVote>& votes);
    
    std::string getFinalDecision();
    
    std::vector<std::string> getConflictingAgents();
    
private:
    ConsensusEngine();
    
    std::string final_decision_;
    std::vector<std::string> conflicting_agents_;
};