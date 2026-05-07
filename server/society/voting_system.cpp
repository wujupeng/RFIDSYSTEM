#include "voting_system.h"

uint64_t VotingSystem::createProposal(const std::string& title,
                                     const std::string& description,
                                     const std::string& proposer) {
    Proposal proposal;
    proposal.proposal_id = next_proposal_id_++;
    proposal.title = title;
    proposal.description = description;
    proposal.proposer = proposer;
    proposal.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    
    proposals_[proposal.proposal_id] = proposal;
    return proposal.proposal_id;
}

bool VotingSystem::castVote(uint64_t proposal_id,
                            const std::string& voter,
                            VoteOption option,
                            const std::string& reason) {
    auto it = proposals_.find(proposal_id);
    if (it == proposals_.end()) {
        return false;
    }
    
    for (const auto& vote : it->second.votes) {
        if (vote.voter == voter) {
            return false;
        }
    }
    
    Vote vote;
    vote.vote_id = rand();
    vote.voter = voter;
    vote.option = option;
    vote.reason = reason;
    vote.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    
    it->second.votes.push_back(vote);
    return true;
}

std::map<VoteOption, int> VotingSystem::getVoteResults(uint64_t proposal_id) {
    std::map<VoteOption, int> results;
    results[VoteOption::YES] = 0;
    results[VoteOption::NO] = 0;
    results[VoteOption::ABSTAIN] = 0;
    
    auto it = proposals_.find(proposal_id);
    if (it == proposals_.end()) {
        return results;
    }
    
    for (const auto& vote : it->second.votes) {
        results[vote.option]++;
    }
    
    return results;
}

bool VotingSystem::isProposalPassed(uint64_t proposal_id) {
    auto results = getVoteResults(proposal_id);
    
    int yes = results[VoteOption::YES];
    int no = results[VoteOption::NO];
    int total = yes + no;
    
    if (total == 0) return false;
    
    return (yes * 100 / total) > 50;
}

Proposal VotingSystem::getProposal(uint64_t proposal_id) {
    auto it = proposals_.find(proposal_id);
    if (it != proposals_.end()) {
        return it->second;
    }
    return Proposal();
}

std::vector<Proposal> VotingSystem::getActiveProposals() {
    std::vector<Proposal> result;
    for (const auto& pair : proposals_) {
        result.push_back(pair.second);
    }
    return result;
}