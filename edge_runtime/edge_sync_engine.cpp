#include "edge_sync_engine.h"

namespace edge_runtime {

EdgeSyncEngine& EdgeSyncEngine::instance() {
    static EdgeSyncEngine engine;
    return engine;
}

void EdgeSyncEngine::initialize() {
    pending_sync_.clear();
    status_ = SyncStatus::SYNCED;
}

void EdgeSyncEngine::shutdown() {
    pending_sync_.clear();
}

void EdgeSyncEngine::tick() {
    if (status_ == SyncStatus::SYNCED && !pending_sync_.empty()) {
        sync();
    }
}

void EdgeSyncEngine::addSyncData(uint64_t timestamp, const std::string& data) {
    SyncEntry entry;
    entry.entry_id = pending_sync_.size() + 1;
    entry.timestamp = timestamp;
    entry.data = data;
    entry.status = SyncStatus::PENDING;
    
    pending_sync_.push_back(entry);
    status_ = SyncStatus::PENDING;
}

void EdgeSyncEngine::sync() {
    if (pending_sync_.empty()) {
        status_ = SyncStatus::SYNCED;
        return;
    }
    
    status_ = SyncStatus::SYNCING;
    processPending();
}

SyncStatus EdgeSyncEngine::getStatus() const {
    return status_;
}

size_t EdgeSyncEngine::getPendingCount() const {
    return pending_sync_.size();
}

void EdgeSyncEngine::clearPending() {
    pending_sync_.clear();
    status_ = SyncStatus::SYNCED;
}

void EdgeSyncEngine::processPending() {
    for (auto& entry : pending_sync_) {
        entry.status = SyncStatus::SYNCED;
    }
    
    pending_sync_.clear();
    status_ = SyncStatus::SYNCED;
}

} // namespace edge_runtime