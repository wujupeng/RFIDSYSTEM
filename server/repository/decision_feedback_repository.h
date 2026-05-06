#pragma once
#include <string>

namespace repository {

class DecisionFeedbackRepository {
public:
    static DecisionFeedbackRepository& instance();

    void insert(int decision_id, int asset_id, bool executed, bool ignored, int user_id = 0);
    double getAdoptionRate(int hours = 24);
    int getTotalFeedbacks(int hours = 24);
    int getExecutedCount(int hours = 24);

private:
    DecisionFeedbackRepository() = default;
    DecisionFeedbackRepository(const DecisionFeedbackRepository&) = delete;
    DecisionFeedbackRepository& operator=(const DecisionFeedbackRepository&) = delete;
};

} // namespace repository
