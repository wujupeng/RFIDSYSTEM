#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <map>

enum class VoteOption {
    YES,
    NO,
    ABSTAIN
};

struct Vote {
    uint64_t vote_id;
    std::string voter;
    VoteOption option;
    std::string reason;
    uint64_t timestamp;
};

struct Proposal {
    uint64_t proposal_id;
    std::string title;
    std::string description;
    std::string proposer;
    uint64_t timestamp;
    std::vector<Vote> votes;
};

class VotingSystem {
public:
    uint64_t createProposal(const std::string& title, 
                           const std::string& description, 
                           const std::string& proposer);
    
    bool castVote(uint64_t proposal_id, 
                  const std::string& voter, 
                  VoteOption option,
                  const std::string& reason = "");
    
    std::map<VoteOption, int> getVoteResults(uint64_t proposal_id);
    
    bool isProposalPassed(uint64_t proposal_id);
    
    Proposal getProposal(uint64_t proposal_id);
    
    std::vector<Proposal> getActiveProposals();
    
private:
    std::map<uint64_t, Proposal> proposals_;
    uint64_t next_proposal_id_ = 1;
};