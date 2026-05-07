#pragma once

#include <cstdint>
#include <vector>
#include <mutex>
#include <fstream>

struct PolicyRecord {
    uint64_t record_id;
    uint64_t timestamp;
    
    uint64_t asset_id;
    int32_t zone_x;
    int32_t zone_y;
    
    float state_vector[16];
    size_t state_dim;
    
    uint32_t action;
    float action_probability;
    
    float reward;
    float cumulative_reward;
    
    float next_state_vector[16];
    size_t next_state_dim;
    
    float human_feedback;
    bool is_human_override;
    
    PolicyRecord()
        : record_id(0), timestamp(0), asset_id(0), zone_x(0), zone_y(0),
          state_dim(0), action(0), action_probability(0.0f),
          reward(0.0f), cumulative_reward(0.0f), next_state_dim(0),
          human_feedback(0.0f), is_human_override(false) {}
};

class PolicyDataset {
public:
    static PolicyDataset& instance();
    
    void initialize();
    void shutdown();
    
    void record(const PolicyRecord& record);
    
    void addHumanFeedback(uint64_t recordId, float feedback);
    void markHumanOverride(uint64_t recordId);
    
    size_t size() const;
    bool empty() const;
    
    std::vector<PolicyRecord> getRecords(size_t offset, size_t count) const;
    std::vector<PolicyRecord> getRecentRecords(size_t count) const;
    
    bool exportToFile(const std::string& filename) const;
    bool loadFromFile(const std::string& filename);
    
    void clear();
    
    size_t getHumanFeedbackCount() const;
    size_t getHumanOverrideCount() const;
    
private:
    PolicyDataset();
    
    std::vector<PolicyRecord> records_;
    mutable std::mutex mutex_;
    
    std::atomic<uint64_t> next_record_id_;
    
    static constexpr size_t MAX_RECORDS = 1000000;
    static constexpr size_t FLUSH_THRESHOLD = 10000;
};