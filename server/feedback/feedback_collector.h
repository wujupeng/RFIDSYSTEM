#pragma once
#include <string>
#include <vector>
#include "../models/decision_feedback.h"

namespace v33 {

class FeedbackCollector {
public:
    static FeedbackCollector& instance();

    void record(const DecisionFeedback& feedback);
    void recordHumanFeedback(int decision_id, const std::string& user_id,
                            const std::string& selected_action, bool was_top1,
                            int response_time_ms, double confidence_shown);

    size_t getPendingCount() const { return pending_feedback_.size(); }

private:
    FeedbackCollector() = default;
    void persistToDatabase(const DecisionFeedback& feedback);

    std::vector<DecisionFeedback> pending_feedback_;
};

}
