#include "proof_ledger.h"
#include <algorithm>

ProofLedger::ProofLedger() {
}

ProofLedger& ProofLedger::instance() {
    static ProofLedger instance;
    return instance;
}

void ProofLedger::initialize() {
    std::lock_guard<std::mutex> lock(mutex_);
    records_.clear();
}

void ProofLedger::shutdown() {
    std::lock_guard<std::mutex> lock(mutex_);
    records_.clear();
}

void ProofLedger::append(const ProofRecord& record) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = std::find_if(records_.begin(), records_.end(),
                          [record](const ProofRecord& r) { return r.frame_id == record.frame_id; });
    
    if (it != records_.end()) {
        *it = record;
    } else {
        records_.push_back(record);
    }
    
    std::sort(records_.begin(), records_.end(),
              [](const ProofRecord& a, const ProofRecord& b) { return a.frame_id < b.frame_id; });
}

bool ProofLedger::verifyChain(uint64_t fromFrameId, uint64_t toFrameId) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto fromIt = std::find_if(records_.begin(), records_.end(),
                              [fromFrameId](const ProofRecord& r) { return r.frame_id == fromFrameId; });
    
    auto toIt = std::find_if(records_.begin(), records_.end(),
                            [toFrameId](const ProofRecord& r) { return r.frame_id == toFrameId; });
    
    if (fromIt == records_.end() || toIt == records_.end()) {
        return false;
    }
    
    for (auto it = fromIt; it != toIt; ++it) {
        if (!it->verified) {
            return false;
        }
    }
    
    return true;
}

bool ProofLedger::verifyFrame(uint64_t frameId) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = std::find_if(records_.begin(), records_.end(),
                          [frameId](const ProofRecord& r) { return r.frame_id == frameId; });
    
    if (it == records_.end()) {
        return false;
    }
    
    return it->verified;
}

std::vector<ProofRecord> ProofLedger::query(uint64_t frameId) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::vector<ProofRecord> result;
    
    auto it = std::find_if(records_.begin(), records_.end(),
                          [frameId](const ProofRecord& r) { return r.frame_id == frameId; });
    
    if (it != records_.end()) {
        result.push_back(*it);
    }
    
    return result;
}

std::vector<ProofRecord> ProofLedger::getRange(uint64_t startFrameId, uint64_t endFrameId) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::vector<ProofRecord> result;
    
    for (const auto& record : records_) {
        if (record.frame_id >= startFrameId && record.frame_id <= endFrameId) {
            result.push_back(record);
        }
    }
    
    return result;
}

bool ProofLedger::verifyFullChain() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    for (const auto& record : records_) {
        if (!record.verified) {
            return false;
        }
    }
    
    return true;
}

size_t ProofLedger::getRecordCount() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return records_.size();
}

void ProofLedger::clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    records_.clear();
}