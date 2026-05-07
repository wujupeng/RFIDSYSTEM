#include "consensus_engine.h"
#include <algorithm>
#include <map>

ConsensusEngine::ConsensusEngine() {}

ConsensusEngine& ConsensusEngine::instance() {
    static ConsensusEngine instance;
    return instance;
}

ConsensusResult ConsensusEngine::reachConsensus(const ConsensusProposal& proposal,
                                               const std::vector<ConsensusVote>& votes) {
    if (votes.empty()) {
        return ConsensusResult::ABSTAINED;
    }
    
    std::map<std::string, double> action_scores;
    
    for (const auto& vote : votes) {
        action_scores[vote.preferred_action] += vote.confidence;
    }
    
    std::vector<std::pair<std::string, double>> sorted_actions(
        action_scores.begin(), action_scores.end());
    
    std::sort(sorted_actions.rbegin(), sorted_actions.rend(),
        [](const std::pair<std::string, double>& a, 
           const std::pair<std::string, double>& b) {
            return a.second > b.second;
        });
    
    if (sorted_actions.size() >= 2 && 
        sorted_actions[0].second - sorted_actions[1].second < 0.1) {
        
        conflicting_agents_.clear();
        for (const auto& vote : votes) {
            if (vote.preferred_action != sorted_actions[0].first) {
                conflicting_agents_.push_back(vote.agent);
            }
        }
        return ConsensusResult::NO_CONSENSUS;
    }
    
    final_decision_ = sorted_actions[0].first;
    return ConsensusResult::CONSENSUS_REACHED;
}

std::string ConsensusEngine::getFinalDecision() {
    return final_decision_;
}

std::vector<std::string> ConsensusEngine::getConflictingAgents() {
    return conflicting_agents_;
}