#include "compute_allocator.h"
#include "runtime_budget.h"

ComputeAllocator::ComputeAllocator() {}

ComputeAllocator& ComputeAllocator::instance() {
    static ComputeAllocator instance;
    return instance;
}

uint64_t ComputeAllocator::allocate(const ComputeRequest& request) {
    auto& budget = RuntimeBudget::instance();
    
    if (!budget.allocateResource(request.resource_type, request.amount)) {
        return 0;
    }
    
    Allocation allocation;
    allocation.allocation_id = next_allocation_id_++;
    allocation.requester = request.requester;
    allocation.resource_type = request.resource_type;
    allocation.amount = request.amount;
    allocation.granted_at = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    
    allocations_[allocation.allocation_id] = allocation;
    return allocation.allocation_id;
}

bool ComputeAllocator::release(uint64_t allocation_id) {
    auto it = allocations_.find(allocation_id);
    if (it == allocations_.end()) {
        return false;
    }
    
    auto& budget = RuntimeBudget::instance();
    budget.releaseResource(it->second.resource_type, it->second.amount);
    
    allocations_.erase(it);
    return true;
}

std::vector<Allocation> ComputeAllocator::getAllocationsFor(const std::string& requester) {
    std::vector<Allocation> result;
    for (const auto& pair : allocations_) {
        if (pair.second.requester == requester) {
            result.push_back(pair.second);
        }
    }
    return result;
}

bool ComputeAllocator::hasAllocation(const std::string& requester, const std::string& resource_type) {
    for (const auto& pair : allocations_) {
        if (pair.second.requester == requester && 
            pair.second.resource_type == resource_type) {
            return true;
        }
    }
    return false;
}