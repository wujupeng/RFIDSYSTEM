#include "decision_feedback.h"
#include <thread>
#include <algorithm>

DecisionFeedbackManager::DecisionFeedbackManager()
    : next_feedback_id_(1), running_(false), processing_(false) {
}

DecisionFeedbackManager::~DecisionFeedbackManager() {
    stopProcessing();
}

DecisionFeedbackManager& DecisionFeedbackManager::instance() {
    static DecisionFeedbackManager instance;
    return instance;
}

uint64_t DecisionFeedbackManager::reportFeedback(uint64_t assetId, uint64_t decisionId,
                                                 uint32_t originalAction, FeedbackAction feedbackAction,
                                                 const std::string& operatorId,
                                                 const std::string& notes) {
    DecisionFeedback feedback;
    feedback.feedback_id = next_feedback_id_++;
    feedback.asset_id = assetId;
    feedback.decision_id = decisionId;
    feedback.original_action = originalAction;
    feedback.feedback_action = feedbackAction;
    feedback.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    
    if (!operatorId.empty()) {
        strncpy(feedback.operator_id, operatorId.c_str(), 31);
    }
    if (!notes.empty()) {
        strncpy(feedback.notes, notes.c_str(), 255);
    }
    
    {
        std::lock_guard<std::mutex> lock(pending_mutex_);
        pending_feedbacks_.push_back(feedback);
    }
    
    cv_.notify_one();
    
    return feedback.feedback_id;
}

bool DecisionFeedbackManager::getFeedback(uint64_t feedbackId, DecisionFeedback& feedback) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = std::find_if(feedbacks_.begin(), feedbacks_.end(),
                          [feedbackId](const DecisionFeedback& f) {
                              return f.feedback_id == feedbackId;
                          });
    
    if (it != feedbacks_.end()) {
        feedback = *it;
        return true;
    }
    
    return false;
}

std::vector<DecisionFeedback> DecisionFeedbackManager::getFeedbacksByAsset(uint64_t assetId) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::vector<DecisionFeedback> result;
    for (const auto& feedback : feedbacks_) {
        if (feedback.asset_id == assetId) {
            result.push_back(feedback);
        }
    }
    
    return result;
}

std::vector<DecisionFeedback> DecisionFeedbackManager::getFeedbacksByTimeRange(uint64_t startTime, uint64_t endTime) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::vector<DecisionFeedback> result;
    for (const auto& feedback : feedbacks_) {
        if (feedback.timestamp >= startTime && feedback.timestamp <= endTime) {
            result.push_back(feedback);
        }
    }
    
    return result;
}

size_t DecisionFeedbackManager::getPendingCount() const {
    std::lock_guard<std::mutex> lock(pending_mutex_);
    return pending_feedbacks_.size();
}

void DecisionFeedbackManager::flushPending() {
    std::lock_guard<std::mutex> pending_lock(pending_mutex_);
    
    if (pending_feedbacks_.empty()) {
        return;
    }
    
    {
        std::lock_guard<std::mutex> lock(mutex_);
        
        for (const auto& feedback : pending_feedbacks_) {
            if (feedbacks_.size() >= MAX_FEEDBACKS) {
                feedbacks_.erase(feedbacks_.begin());
            }
            feedbacks_.push_back(feedback);
        }
    }
    
    pending_feedbacks_.clear();
}

void DecisionFeedbackManager::startProcessing() {
    if (running_) {
        return;
    }
    
    running_ = true;
    
    std::thread t(&DecisionFeedbackManager::processingLoop, this);
    t.detach();
}

void DecisionFeedbackManager::stopProcessing() {
    running_ = false;
    cv_.notify_one();
    
    while (processing_) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void DecisionFeedbackManager::processingLoop() {
    processing_ = true;
    
    while (running_) {
        std::unique_lock<std::mutex> lock(pending_mutex_);
        
        cv_.wait(lock, [this] {
            return !pending_feedbacks_.empty() || !running_;
        });
        
        if (!running_) {
            break;
        }
        
        if (pending_feedbacks_.size() >= FLUSH_THRESHOLD) {
            lock.unlock();
            flushPending();
            lock.lock();
        }
    }
    
    processing_ = false;
}