#pragma once
#include <cstdint>

namespace v33 {

struct DecisionOutcome {
    int decision_id;

    bool actual_missing;
    bool actual_issue;

    int verified_after_hours;

    int64_t timestamp;
};

}
