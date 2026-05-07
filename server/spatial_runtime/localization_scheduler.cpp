#include "localization_scheduler.h"
#include <algorithm>

LocalizationScheduler::LocalizationScheduler() : worker_pool_(4) {}

LocalizationScheduler& LocalizationScheduler::instance() {
    static LocalizationScheduler instance;
    return instance;
}

void LocalizationScheduler::initialize() {
    worker_pool_.start();
}

void LocalizationScheduler::shutdown() {
    worker_pool_.stop();
}

void LocalizationScheduler::tick() {
    budget_.reset();
    processPending();
    applyDegradation();
}

void LocalizationScheduler::scheduleObservation(const TagObservation& observation) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (last_positions_.size() >= max_tags_) {
        return;
    }
    
    pending_observations_.push_back(observation);
}

void LocalizationScheduler::processPending() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    for (const auto& obs : pending_observations_) {
        LocalizationTask task;
        task.epc = obs.epc;
        task.observations.push_back(obs);
        task.callback = [this](TagPosition pos) {
            std::lock_guard<std::mutex> lock(mutex_);
            last_positions_[pos.x > 0 ? "tag" : ""] = pos;
        };
        
        worker_pool_.submitTask(task);
    }
    
    pending_observations_.clear();
}

void LocalizationScheduler::applyDegradation() {
    if (budget_.isOverBudget()) {
        pipeline_.setStageEnabled(PipelineStage::KALMAN_FILTERING, false);
    } else {
        pipeline_.setStageEnabled(PipelineStage::KALMAN_FILTERING, true);
    }
}

TagPosition LocalizationScheduler::getPosition(const std::string& epc) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = last_positions_.find(epc);
    if (it != last_positions_.end()) {
        return it->second;
    }
    
    return TagPosition();
}

size_t LocalizationScheduler::getTrackedCount() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return last_positions_.size();
}

void LocalizationScheduler::setMaxTags(size_t max) {
    max_tags_ = max;
}