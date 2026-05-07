#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <mutex>

struct ProofRecord {
    uint64_t frame_id;
    
    uint64_t frame_hash;
    uint64_t gpu_hash;
    uint64_t ai_hash;
    
    uint64_t decision_hash;
    
    uint64_t recovery_hash;
    
    bool verified;
    
    std::string signature;
    
    uint64_t timestamp;
    
    ProofRecord() 
        : frame_id(0), frame_hash(0), gpu_hash(0), ai_hash(0),
          decision_hash(0), recovery_hash(0), verified(false),
          timestamp(0) {}
};

class ProofLedger {
public:
    static ProofLedger& instance();
    
    void initialize();
    void shutdown();
    
    void append(const ProofRecord& record);
    
    bool verifyChain(uint64_t fromFrameId, uint64_t toFrameId);
    
    bool verifyFrame(uint64_t frameId);
    
    std::vector<ProofRecord> query(uint64_t frameId);
    
    std::vector<ProofRecord> getRange(uint64_t startFrameId, uint64_t endFrameId);
    
    bool verifyFullChain();
    
    size_t getRecordCount() const;
    
    void clear();
    
private:
    ProofLedger();
    
    std::vector<ProofRecord> records_;
    mutable std::mutex mutex_;
};