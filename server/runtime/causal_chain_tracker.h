#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <mutex>
#include <unordered_map>

struct CausalNode {
    uint64_t id;
    
    std::string type;
    
    std::vector<uint64_t> parents;
    
    std::string data_hash;
    
    uint64_t timestamp;
    
    CausalNode() 
        : id(0), timestamp(0) {}
};

class CausalChainTracker {
public:
    static CausalChainTracker& instance();
    
    void initialize();
    void shutdown();
    
    void addNode(uint64_t id, const std::string& type, 
                const std::vector<uint64_t>& parents, const std::string& dataHash);
    
    std::vector<CausalNode> traceDecision(uint64_t decisionId);
    
    std::vector<uint64_t> explainFailure(uint64_t frameId);
    
    std::vector<CausalNode> getNodeChain(uint64_t nodeId);
    
    bool verifyChain(const std::vector<CausalNode>& chain);
    
    size_t getNodeCount() const;
    
private:
    CausalChainTracker();
    
    std::unordered_map<uint64_t, CausalNode> nodes_;
    
    mutable std::mutex mutex_;
};