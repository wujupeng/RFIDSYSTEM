#include "policy_dataset.h"
#include <algorithm>
#include <cstring>

PolicyDataset::PolicyDataset()
    : next_record_id_(1) {
}

PolicyDataset& PolicyDataset::instance() {
    static PolicyDataset instance;
    return instance;
}

void PolicyDataset::initialize() {
    std::lock_guard<std::mutex> lock(mutex_);
    records_.clear();
    next_record_id_ = 1;
}

void PolicyDataset::shutdown() {
    std::lock_guard<std::mutex> lock(mutex_);
    records_.clear();
}

void PolicyDataset::record(const PolicyRecord& record) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (records_.size() >= MAX_RECORDS) {
        records_.erase(records_.begin());
    }
    
    PolicyRecord newRecord = record;
    newRecord.record_id = next_record_id_++;
    
    records_.push_back(newRecord);
}

void PolicyDataset::addHumanFeedback(uint64_t recordId, float feedback) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = std::find_if(records_.begin(), records_.end(),
                          [recordId](const PolicyRecord& r) { return r.record_id == recordId; });
    
    if (it != records_.end()) {
        it->human_feedback = feedback;
    }
}

void PolicyDataset::markHumanOverride(uint64_t recordId) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = std::find_if(records_.begin(), records_.end(),
                          [recordId](const PolicyRecord& r) { return r.record_id == recordId; });
    
    if (it != records_.end()) {
        it->is_human_override = true;
    }
}

size_t PolicyDataset::size() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return records_.size();
}

bool PolicyDataset::empty() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return records_.empty();
}

std::vector<PolicyRecord> PolicyDataset::getRecords(size_t offset, size_t count) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::vector<PolicyRecord> result;
    
    size_t start = std::min(offset, records_.size());
    size_t end = std::min(offset + count, records_.size());
    
    for (size_t i = start; i < end; ++i) {
        result.push_back(records_[i]);
    }
    
    return result;
}

std::vector<PolicyRecord> PolicyDataset::getRecentRecords(size_t count) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::vector<PolicyRecord> result;
    
    size_t start = count >= records_.size() ? 0 : records_.size() - count;
    
    for (size_t i = start; i < records_.size(); ++i) {
        result.push_back(records_[i]);
    }
    
    return result;
}

bool PolicyDataset::exportToFile(const std::string& filename) const {
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    uint64_t count = records_.size();
    file.write(reinterpret_cast<const char*>(&count), sizeof(uint64_t));
    
    for (const auto& record : records_) {
        file.write(reinterpret_cast<const char*>(&record.record_id), sizeof(uint64_t));
        file.write(reinterpret_cast<const char*>(&record.timestamp), sizeof(uint64_t));
        file.write(reinterpret_cast<const char*>(&record.asset_id), sizeof(uint64_t));
        file.write(reinterpret_cast<const char*>(&record.zone_x), sizeof(int32_t));
        file.write(reinterpret_cast<const char*>(&record.zone_y), sizeof(int32_t));
        
        file.write(reinterpret_cast<const char*>(&record.state_dim), sizeof(size_t));
        file.write(reinterpret_cast<const char*>(record.state_vector), sizeof(float) * 16);
        
        file.write(reinterpret_cast<const char*>(&record.action), sizeof(uint32_t));
        file.write(reinterpret_cast<const char*>(&record.action_probability), sizeof(float));
        file.write(reinterpret_cast<const char*>(&record.reward), sizeof(float));
        file.write(reinterpret_cast<const char*>(&record.cumulative_reward), sizeof(float));
        
        file.write(reinterpret_cast<const char*>(&record.next_state_dim), sizeof(size_t));
        file.write(reinterpret_cast<const char*>(record.next_state_vector), sizeof(float) * 16);
        
        file.write(reinterpret_cast<const char*>(&record.human_feedback), sizeof(float));
        file.write(reinterpret_cast<const char*>(&record.is_human_override), sizeof(bool));
    }
    
    file.close();
    return true;
}

bool PolicyDataset::loadFromFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }
    
    uint64_t count = 0;
    file.read(reinterpret_cast<char*>(&count), sizeof(uint64_t));
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    records_.clear();
    
    for (uint64_t i = 0; i < count; ++i) {
        PolicyRecord record;
        
        file.read(reinterpret_cast<char*>(&record.record_id), sizeof(uint64_t));
        file.read(reinterpret_cast<char*>(&record.timestamp), sizeof(uint64_t));
        file.read(reinterpret_cast<char*>(&record.asset_id), sizeof(uint64_t));
        file.read(reinterpret_cast<char*>(&record.zone_x), sizeof(int32_t));
        file.read(reinterpret_cast<char*>(&record.zone_y), sizeof(int32_t));
        
        file.read(reinterpret_cast<char*>(&record.state_dim), sizeof(size_t));
        file.read(reinterpret_cast<char*>(record.state_vector), sizeof(float) * 16);
        
        file.read(reinterpret_cast<char*>(&record.action), sizeof(uint32_t));
        file.read(reinterpret_cast<char*>(&record.action_probability), sizeof(float));
        file.read(reinterpret_cast<char*>(&record.reward), sizeof(float));
        file.read(reinterpret_cast<char*>(&record.cumulative_reward), sizeof(float));
        
        file.read(reinterpret_cast<char*>(&record.next_state_dim), sizeof(size_t));
        file.read(reinterpret_cast<char*>(record.next_state_vector), sizeof(float) * 16);
        
        file.read(reinterpret_cast<char*>(&record.human_feedback), sizeof(float));
        file.read(reinterpret_cast<char*>(&record.is_human_override), sizeof(bool));
        
        records_.push_back(record);
    }
    
    file.close();
    
    if (!records_.empty()) {
        next_record_id_ = records_.back().record_id + 1;
    }
    
    return true;
}

void PolicyDataset::clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    records_.clear();
}

size_t PolicyDataset::getHumanFeedbackCount() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    size_t count = 0;
    for (const auto& record : records_) {
        if (record.human_feedback != 0.0f) {
            count++;
        }
    }
    
    return count;
}

size_t PolicyDataset::getHumanOverrideCount() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    size_t count = 0;
    for (const auto& record : records_) {
        if (record.is_human_override) {
            count++;
        }
    }
    
    return count;
}