#include "feedback_collector.h"
#include "../core/logger.h"
#include "../db/db_pool.h"
#include <chrono>

namespace v33 {

FeedbackCollector& FeedbackCollector::instance() {
    static FeedbackCollector instance;
    return instance;
}

void FeedbackCollector::record(const DecisionFeedback& feedback) {
    spdlog::debug("FeedbackCollector: Recording feedback for decision {}",
                  feedback.decision_id);

    pending_feedback_.push_back(feedback);

    if (pending_feedback_.size() >= 100) {
        for (const auto& fb : pending_feedback_) {
            persistToDatabase(fb);
        }
        pending_feedback_.clear();
    }
}

void FeedbackCollector::recordHumanFeedback(
    int decision_id,
    const std::string& user_id,
    const std::string& selected_action,
    bool was_top1,
    int response_time_ms,
    double confidence_shown
) {
    DecisionFeedback feedback;
    feedback.decision_id = decision_id;
    feedback.user_id = user_id;
    feedback.selected_action = selected_action;
    feedback.was_top1 = was_top1;
    feedback.response_time_ms = response_time_ms;
    feedback.confidence_shown = confidence_shown;
    feedback.executed = false;
    feedback.ignored = false;
    feedback.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();

    record(feedback);

    spdlog::info("FeedbackCollector: Human feedback recorded for decision {} by user {}",
                 decision_id, user_id);
}

void FeedbackCollector::persistToDatabase(const DecisionFeedback& feedback) {
    try {
        auto db = DBPool::instance().acquire();
        pqxx::work txn(*db);

        txn.exec_params(
            R"(
                INSERT INTO decision_feedback
                (decision_id, user_id, selected_action, was_top1, response_time_ms,
                 confidence_shown, executed, ignored, timestamp)
                VALUES ($1, $2, $3, $4, $5, $6, $7, $8, $9)
            )",
            feedback.decision_id,
            feedback.user_id,
            feedback.selected_action,
            feedback.was_top1,
            feedback.response_time_ms,
            feedback.confidence_shown,
            feedback.executed,
            feedback.ignored,
            feedback.timestamp
        );

        txn.commit();
        spdlog::debug("FeedbackCollector: Persisted feedback for decision {}",
                      feedback.decision_id);
    } catch (const std::exception& e) {
        spdlog::error("FeedbackCollector: Failed to persist feedback - {}",
                      e.what());
    }
}

}
