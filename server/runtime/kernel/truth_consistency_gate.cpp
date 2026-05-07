#include "truth_consistency_gate.h"

namespace runtime {

TruthConsistencyGate::TruthConsistencyGate() {}

int TruthConsistencyGate::evaluate(const ConsistencyInput& input) {
    last_result_ = checker_.verify(input);

    int score = 0;
    if (last_result_.frame_consistent) score += 34;
    if (last_result_.ai_consistent) score += 33;
    if (last_result_.gpu_consistent) score += 33;

    last_score_ = score;
    return score;
}

const ConsistencyResult& TruthConsistencyGate::getLastResult() const {
    return last_result_;
}

bool TruthConsistencyGate::isValid() const {
    return last_result_.frame_consistent && 
           last_result_.ai_consistent && 
           last_result_.gpu_consistent;
}

} // namespace runtime