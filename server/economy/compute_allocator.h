#pragma once

#include <cstdint>
#include <string>
#include <map>

struct ComputeRequest {
    std::string requester;
    std::string resource_type;
    double amount;
    double priority;
    uint64_t timestamp;
};

struct Allocation {
    uint64_t allocation_id;
    std::string requester;
    std::string resource_type;
    double amount;
    uint64_t granted_at;
};

class ComputeAllocator {
public:
    static ComputeAllocator& instance();
    
    uint64_t allocate(const ComputeRequest& request);
    
    bool release(uint64_t allocation_id);
    
    std::vector<Allocation> getAllocationsFor(const std::string& requester);
    
    bool hasAllocation(const std::string& requester, const std::string& resource_type);
    
private:
    ComputeAllocator();
    
    std::map<uint64_t, Allocation> allocations_;
    uint64_t next_allocation_id_ = 1;
};