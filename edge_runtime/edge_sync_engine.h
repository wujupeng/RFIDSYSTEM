#pragma once

#include <cstdint>
#include <vector>
#include <map>

namespace edge_runtime {

enum class SyncStatus {
    SYNCED,
    PENDING,
    SYNCING,
    FAILED
};

struct SyncEntry {
    uint64_t entry_id;
    uint64_t timestamp;
    std::string data;
    SyncStatus status;
};

class EdgeSyncEngine {
public:
    static EdgeSyncEngine& instance();
    
    void initialize();
    
    void shutdown();
    
    void tick();
    
    void addSyncData(uint64_t timestamp, const std::string& data);
    
    void sync();
    
    SyncStatus getStatus() const;
    
    size_t getPendingCount() const;
    
    void clearPending();
    
private:
    EdgeSyncEngine() = default;
    
    void processPending();
    
    std::vector<SyncEntry> pending_sync_;
    SyncStatus status_ = SyncStatus::SYNCED;
};

} // namespace edge_runtime