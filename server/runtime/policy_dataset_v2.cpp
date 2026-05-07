#include "policy_dataset_v2.h"
#include <algorithm>

PolicyDatasetV2::PolicyDatasetV2()
    : next_episode_id_(1) {
}

PolicyDatasetV2& PolicyDatasetV2::instance() {
    static PolicyDatasetV2 instance;
    return instance;
}

void PolicyDatasetV2::initialize() {
    std::lock_guard<std::mutex> lock(mutex_);
    episodes_.clear();
    next_episode_id_ = 1;
}

void PolicyDatasetV2::shutdown() {
    std::lock_guard<std::mutex> lock(mutex_);
    episodes_.clear();
}

uint64_t PolicyDatasetV2::startEpisode() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    Episode episode;
    episode.episode_id = next_episode_id_++;
    episode.start_timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    episode.is_complete = false;
    
    episodes_.push_back(episode);
    
    return episode.episode_id;
}

void PolicyDatasetV2::endEpisode(uint64_t episodeId, bool isComplete) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = std::find_if(episodes_.begin(), episodes_.end(),
                          [episodeId](const Episode& e) { return e.episode_id == episodeId; });
    
    if (it != episodes_.end()) {
        it->is_complete = isComplete;
        it->end_timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        
        finalizeEpisode(episodeId);
    }
}

void PolicyDatasetV2::addTransition(uint64_t episodeId, const FrameState& state, 
                                   const Action& action, float reward, float humanFeedback) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = std::find_if(episodes_.begin(), episodes_.end(),
                          [episodeId](const Episode& e) { return e.episode_id == episodeId; });
    
    if (it != episodes_.end()) {
        it->states.push_back(state);
        it->actions.push_back(action);
        
        float adjustedReward = reward + humanFeedback * HUMAN_FEEDBACK_WEIGHT;
        it->rewards.push_back(adjustedReward);
        it->human_feedbacks.push_back(humanFeedback);
        
        it->total_reward += adjustedReward;
    }
}

void PolicyDatasetV2::finalizeEpisode(uint64_t episodeId) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = std::find_if(episodes_.begin(), episodes_.end(),
                          [episodeId](const Episode& e) { return e.episode_id == episodeId; });
    
    if (it != episodes_.end()) {
        if (!it->actions.empty()) {
            float sumConfidence = 0.0f;
            for (const auto& action : it->actions) {
                sumConfidence += action.confidence;
            }
            it->average_confidence = sumConfidence / it->actions.size();
        }
    }
}

std::vector<Episode> PolicyDatasetV2::getEpisodes() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return episodes_;
}

Episode PolicyDatasetV2::getEpisode(uint64_t episodeId) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = std::find_if(episodes_.begin(), episodes_.end(),
                          [episodeId](const Episode& e) { return e.episode_id == episodeId; });
    
    if (it != episodes_.end()) {
        return *it;
    }
    
    return Episode();
}

std::vector<SARSDRecord> PolicyDatasetV2::exportSARSDFormat() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::vector<SARSDRecord> records;
    
    for (const auto& episode : episodes_) {
        for (size_t i = 0; i < episode.states.size(); ++i) {
            SARSDRecord record;
            record.state = episode.states[i];
            record.action = episode.actions[i];
            record.reward = episode.rewards[i];
            record.human_feedback = episode.human_feedbacks[i];
            
            if (i + 1 < episode.states.size()) {
                record.next_state = episode.states[i + 1];
                record.done = false;
            } else {
                record.done = episode.is_complete;
            }
            
            records.push_back(record);
        }
    }
    
    return records;
}

bool PolicyDatasetV2::exportToFile(const std::string& filename) const {
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    uint64_t episodeCount = episodes_.size();
    file.write(reinterpret_cast<const char*>(&episodeCount), sizeof(uint64_t));
    
    for (const auto& episode : episodes_) {
        file.write(reinterpret_cast<const char*>(&episode.episode_id), sizeof(uint64_t));
        file.write(reinterpret_cast<const char*>(&episode.start_timestamp), sizeof(uint64_t));
        file.write(reinterpret_cast<const char*>(&episode.end_timestamp), sizeof(uint64_t));
        file.write(reinterpret_cast<const char*>(&episode.is_complete), sizeof(bool));
        
        uint64_t stateCount = episode.states.size();
        file.write(reinterpret_cast<const char*>(&stateCount), sizeof(uint64_t));
        
        for (const auto& state : episode.states) {
            file.write(reinterpret_cast<const char*>(&state.frame_id), sizeof(uint64_t));
            file.write(reinterpret_cast<const char*>(&state.timestamp), sizeof(uint64_t));
            file.write(reinterpret_cast<const char*>(&state.risk_index), sizeof(float));
            file.write(reinterpret_cast<const char*>(&state.congestion), sizeof(float));
            file.write(reinterpret_cast<const char*>(&state.anomaly_score), sizeof(float));
            file.write(reinterpret_cast<const char*>(&state.asset_count), sizeof(uint32_t));
            file.write(reinterpret_cast<const char*>(&state.high_risk_count), sizeof(uint32_t));
        }
        
        for (const auto& action : episode.actions) {
            file.write(reinterpret_cast<const char*>(&action.action_type), sizeof(uint32_t));
            file.write(reinterpret_cast<const char*>(&action.target_asset_id), sizeof(uint64_t));
            file.write(reinterpret_cast<const char*>(&action.confidence), sizeof(float));
        }
        
        for (const auto& reward : episode.rewards) {
            file.write(reinterpret_cast<const char*>(&reward), sizeof(float));
        }
        
        for (const auto& feedback : episode.human_feedbacks) {
            file.write(reinterpret_cast<const char*>(&feedback), sizeof(float));
        }
        
        file.write(reinterpret_cast<const char*>(&episode.total_reward), sizeof(float));
        file.write(reinterpret_cast<const char*>(&episode.average_confidence), sizeof(float));
    }
    
    file.close();
    return true;
}

bool PolicyDatasetV2::loadFromFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }
    
    uint64_t episodeCount = 0;
    file.read(reinterpret_cast<char*>(&episodeCount), sizeof(uint64_t));
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    episodes_.clear();
    
    for (uint64_t i = 0; i < episodeCount; ++i) {
        Episode episode;
        
        file.read(reinterpret_cast<char*>(&episode.episode_id), sizeof(uint64_t));
        file.read(reinterpret_cast<char*>(&episode.start_timestamp), sizeof(uint64_t));
        file.read(reinterpret_cast<char*>(&episode.end_timestamp), sizeof(uint64_t));
        file.read(reinterpret_cast<char*>(&episode.is_complete), sizeof(bool));
        
        uint64_t stateCount = 0;
        file.read(reinterpret_cast<char*>(&stateCount), sizeof(uint64_t));
        
        episode.states.resize(stateCount);
        for (uint64_t j = 0; j < stateCount; ++j) {
            FrameState state;
            file.read(reinterpret_cast<char*>(&state.frame_id), sizeof(uint64_t));
            file.read(reinterpret_cast<char*>(&state.timestamp), sizeof(uint64_t));
            file.read(reinterpret_cast<char*>(&state.risk_index), sizeof(float));
            file.read(reinterpret_cast<char*>(&state.congestion), sizeof(float));
            file.read(reinterpret_cast<char*>(&state.anomaly_score), sizeof(float));
            file.read(reinterpret_cast<char*>(&state.asset_count), sizeof(uint32_t));
            file.read(reinterpret_cast<char*>(&state.high_risk_count), sizeof(uint32_t));
            episode.states[j] = state;
        }
        
        episode.actions.resize(stateCount);
        for (uint64_t j = 0; j < stateCount; ++j) {
            Action action;
            file.read(reinterpret_cast<char*>(&action.action_type), sizeof(uint32_t));
            file.read(reinterpret_cast<char*>(&action.target_asset_id), sizeof(uint64_t));
            file.read(reinterpret_cast<char*>(&action.confidence), sizeof(float));
            episode.actions[j] = action;
        }
        
        episode.rewards.resize(stateCount);
        for (uint64_t j = 0; j < stateCount; ++j) {
            file.read(reinterpret_cast<char*>(&episode.rewards[j]), sizeof(float));
        }
        
        episode.human_feedbacks.resize(stateCount);
        for (uint64_t j = 0; j < stateCount; ++j) {
            file.read(reinterpret_cast<char*>(&episode.human_feedbacks[j]), sizeof(float));
        }
        
        file.read(reinterpret_cast<char*>(&episode.total_reward), sizeof(float));
        file.read(reinterpret_cast<char*>(&episode.average_confidence), sizeof(float));
        
        episodes_.push_back(episode);
    }
    
    file.close();
    
    if (!episodes_.empty()) {
        next_episode_id_ = episodes_.back().episode_id + 1;
    }
    
    return true;
}

size_t PolicyDatasetV2::getEpisodeCount() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return episodes_.size();
}

size_t PolicyDatasetV2::getTotalTransitionCount() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    size_t count = 0;
    for (const auto& episode : episodes_) {
        count += episode.states.size();
    }
    
    return count;
}

float PolicyDatasetV2::getAverageHumanFeedback() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    float sum = 0.0f;
    size_t count = 0;
    
    for (const auto& episode : episodes_) {
        for (float feedback : episode.human_feedbacks) {
            sum += feedback;
            count++;
        }
    }
    
    return count > 0 ? sum / count : 0.0f;
}