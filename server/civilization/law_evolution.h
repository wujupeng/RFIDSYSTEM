#pragma once

#include <cstdint>
#include <string>
#include <vector>

enum class LawChangeType {
    ADD,
    MODIFY,
    REMOVE
};

struct LawRevision {
    uint64_t law_id;
    uint64_t revision;
    uint64_t timestamp;
    LawChangeType change_type;
    std::string description;
    std::string reason;
    std::string author;
};

class LawEvolution {
public:
    void recordLawChange(const LawRevision& revision);
    
    std::vector<LawRevision> getLawHistory(uint64_t law_id);
    
    std::vector<LawRevision> getAllRevisions();
    
    uint64_t getCurrentRevision(uint64_t law_id) const;
    
private:
    std::vector<LawRevision> revisions_;
};