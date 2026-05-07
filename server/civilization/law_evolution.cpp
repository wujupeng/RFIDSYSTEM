#include "law_evolution.h"
#include <algorithm>

void LawEvolution::recordLawChange(const LawRevision& revision) {
    revisions_.push_back(revision);
}

std::vector<LawRevision> LawEvolution::getLawHistory(uint64_t law_id) {
    std::vector<LawRevision> result;
    for (const auto& rev : revisions_) {
        if (rev.law_id == law_id) {
            result.push_back(rev);
        }
    }
    std::sort(result.begin(), result.end(),
        [](const LawRevision& a, const LawRevision& b) { return a.revision < b.revision; });
    return result;
}

std::vector<LawRevision> LawEvolution::getAllRevisions() {
    return revisions_;
}

uint64_t LawEvolution::getCurrentRevision(uint64_t law_id) const {
    uint64_t max_rev = 0;
    for (const auto& rev : revisions_) {
        if (rev.law_id == law_id && rev.revision > max_rev) {
            max_rev = rev.revision;
        }
    }
    return max_rev;
}