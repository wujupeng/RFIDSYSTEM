#pragma once

#include <cstdint>
#include <vector>
#include <mutex>
#include <fstream>

struct FrameState {
    uint64_t frame_id;
    uint64_t timestamp;
    
    float risk_index;
    float congestion;
    float anomaly_score;
    
    uint32_t asset_count;
    uint32_t high_risk_count;
    
    FrameState() 
        : frame_id(0), timestamp(0), risk_index(0.0f), 
          congestion(0.0f), anomaly_score(0.0f),
          asset_count(0), high_risk_count(0) {}
};

struct Action {
    uint32_t action_type;
    uint64_t target_asset_id;
    float confidence;
    
    Action() : action_type(0), target_asset_id(0), confidence(0.0f) {}
};

struct SARSDRecord {
    FrameState state;
    Action action;
    float reward;
    FrameState next_state;
    bool done;
    float human_feedback;
    
    SARSDRecord() : reward(0.0f), done(false), human_feedback(0.0f) {}
};

struct Episode {
    uint64_t episode_id;
    uint64_t start_timestamp;
    uint64_t end_timestamp;
    bool is_complete;
    
    std::vector<FrameState> states;
    std::vector<Action> actions;
    std::vector<float> rewards;
    std::vector<float> human_feedbacks;
    
    float total_reward;
    float average_confidence;
    
    Episode() 
        : episode_id(0), start_timestamp(0), end_timestamp(0), 
          is_complete(false), total_reward(0.0f), average_confidence(0.0f) {}
};

class PolicyDatasetV2 {
public:
    static PolicyDatasetV2& instance();
    
    void initialize();
    void shutdown();
    
    uint64_t startEpisode();
    void endEpisode(uint64_t episodeId, bool isComplete);
    
    void addTransition(uint64_t episodeId, const FrameState& state, 
                      const Action& action, float reward, float humanFeedback = 0.0f);
    
    void finalizeEpisode(uint64_t episodeId);
    
    std::vector<Episode> getEpisodes() const;
    Episode getEpisode(uint64_t episodeId) const;
    
    std::vector<SARSDRecord> exportSARSDFormat() const;
    
    bool exportToFile(const std::string& filename) const;
    bool loadFromFile(const std::string& filename);
    
    size_t getEpisodeCount() const;
    size_t getTotalTransitionCount() const;
    
    float getAverageHumanFeedback() const;
    
private:
    PolicyDatasetV2();
    
    std::vector<Episode> episodes_;
    mutable std::mutex mutex_;
    std::atomic<uint64_t> next_episode_id_;
    
    static constexpr float HUMAN_FEEDBACK_WEIGHT = 0.5f;
};