#pragma once
#include <string>
#include <cstdint>

namespace v33 {

struct DecisionFeedback {
    int decision_id;
    std::string user_id;

    std::string selected_action;
    bool was_top1;

    int response_time_ms;
    double confidence_shown;

    bool executed;
    bool ignored;

    int64_t timestamp;
};

}
