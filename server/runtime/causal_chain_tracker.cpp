#include "causal_chain_tracker.h"
#include <algorithm>

CausalChainTracker::CausalChainTracker() {
}

CausalChainTracker& CausalChainTracker::instance() {
    static CausalChainTracker instance;
    return instance;
}

void CausalChainTracker::initialize() {
    std::lock_guard<std::mutex> lock(mutex_);
    nodes_.clear();
}

void CausalChainTracker::shutdown() {
    std::lock_guard<std::mutex> lock(mutex_);
    nodes_.clear();
}

void CausalChainTracker::addNode(uint64_t id, const std::string& type, 
                                const std::vector<uint64_t>& parents, 
                                const std::string& dataHash) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    CausalNode node;
    node.id = id;
    node.type = type;
    node.parents = parents;
    node.data_hash = dataHash;
    node.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    
    nodes_[id] = node;
}

std::vector<CausalNode> CausalChainTracker::traceDecision(uint64_t decisionId) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::vector<CausalNode> chain;
    
    auto it = nodes_.find(decisionId);
    if (it == nodes_.end()) {
        return chain;
    }
    
    std::unordered_map<uint64_t, bool> visited;
    std::vector<uint64_t> stack;
    stack.push_back(decisionId);
    
    while (!stack.empty()) {
        uint64_t currentId = stack.back();
        stack.pop_back();
        
        if (visited[currentId]) {
            continue;
        }
        
        visited[currentId] = true;
        
        auto nodeIt = nodes_.find(currentId);
        if (nodeIt == nodes_.end()) {
            continue;
        }
        
        chain.push_back(nodeIt->second);
        
        for (uint64_t parentId : nodeIt->second.parents) {
            if (!visited[parentId]) {
                stack.push_back(parentId);
            }
        }
    }
    
    std::reverse(chain.begin(), chain.end());
    
    return chain;
}

std::vector<uint64_t> CausalChainTracker::explainFailure(uint64_t frameId) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::vector<uint64_t> failureChain;
    
    for (const auto& pair : nodes_) {
        const auto& node = pair.second;
        
        if (node.type == "FAILURE" || node.type == "DECISION") {
            for (uint64_t parentId : node.parents) {
                auto parentIt = nodes_.find(parentId);
                if (parentIt != nodes_.end()) {
                    failureChain.push_back(parentId);
                }
            }
        }
    }
    
    return failureChain;
}

std::vector<CausalNode> CausalChainTracker::getNodeChain(uint64_t nodeId) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::vector<CausalNode> chain;
    
    auto it = nodes_.find(nodeId);
    if (it == nodes_.end()) {
        return chain;
    }
    
    chain.push_back(it->second);
    
    return chain;
}

bool CausalChainTracker::verifyChain(const std::vector<CausalNode>& chain) {
    if (chain.empty()) {
        return false;
    }
    
    for (size_t i = 1; i < chain.size(); ++i) {
        const auto& child = chain[i];
        
        bool foundParent = false;
        for (uint64_t parentId : child.parents) {
            if (parentId == chain[i-1].id) {
                foundParent = true;
                break;
            }
        }
        
        if (!foundParent) {
            return false;
        }
    }
    
    return true;
}

size_t CausalChainTracker::getNodeCount() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return nodes_.size();
}