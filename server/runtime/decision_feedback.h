#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <mutex>
#include <atomic>
#include <condition_variable>

enum class FeedbackAction {
    CONFIRM_INSPECT = 1,
    IGNORE = 2,
    CHANGE_TO_ALERT = 3,
    CONFIRM_ALERT = 4,
    ESCALATE = 5
};

struct DecisionFeedback {
    uint64_t feedback_id;
    uint64_t asset_id;
    uint64_t decision_id;
    uint32_t original_action;
    FeedbackAction feedback_action;
    uint64_t timestamp;
    char operator_id[32];
    char notes[256];
    
    DecisionFeedback() 
        : feedback_id(0), asset_id(0), decision_id(0), 
          original_action(0), feedback_action(FeedbackAction::IGNORE),
          timestamp(0) {
        memset(operator_id, 0, 32);
        memset(notes, 0, 256);
    }
};

class DecisionFeedbackManager {
public:
    static DecisionFeedbackManager& instance();
    
    uint64_t reportFeedback(uint64_t assetId, uint64_t decisionId,
                           uint32_t originalAction, FeedbackAction feedbackAction,
                           const std::string& operatorId = "",
                           const std::string& notes = "");
    
    bool getFeedback(uint64_t feedbackId, DecisionFeedback& feedback) const;
    std::vector<DecisionFeedback> getFeedbacksByAsset(uint64_t assetId) const;
    std::vector<DecisionFeedback> getFeedbacksByTimeRange(uint64_t startTime, uint64_t endTime) const;
    
    size_t getPendingCount() const;
    void flushPending();
    
    void startProcessing();
    void stopProcessing();
    
private:
    DecisionFeedbackManager();
    ~DecisionFeedbackManager();
    
    void processingLoop();
    
    std::vector<DecisionFeedback> feedbacks_;
    std::vector<DecisionFeedback> pending_feedbacks_;
    
    std::mutex mutex_;
    std::mutex pending_mutex_;
    std::condition_variable cv_;
    
    std::atomic<uint64_t> next_feedback_id_;
    std::atomic<bool> running_;
    std::atomic<bool> processing_;
    
    static constexpr size_t MAX_FEEDBACKS = 100000;
    static constexpr size_t FLUSH_THRESHOLD = 100;
};