#include "epc_buffer.h"
#include "../db/db_pool.h"
#include "logger.h"
#include <sstream>
#include <thread>

namespace rfid {

EPCBuffer::EPCBuffer()
    : lastCleanupTime_(0), totalProcessed_(0), totalDuplicates_(0) {}

void EPCBuffer::push(const std::string& epc, int task_id, int rssi, const std::string& reader_id) {
    EPCEntry entry;
    entry.epc = epc;
    entry.task_id = task_id;
    entry.rssi = rssi;
    entry.reader_id = reader_id;
    entry.timestamp_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    push(entry);
}

void EPCBuffer::push(const EPCEntry& entry) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    int64_t now_ms = entry.timestamp_ms;
    
    if (isDuplicate(entry.epc, now_ms)) {
        totalDuplicates_++;
        return;
    }
    
    size_t totalSize = 0;
    for (const auto& pair : taskBuffers_) {
        totalSize += pair.second.size();
    }
    
    if (totalSize >= MAX_BUFFER_SIZE) {
        spdlog::warn("EPC buffer full ({}), dropping entry", totalSize);
        return;
    }
    
    taskBuffers_[entry.task_id].push_back(entry);
    recentEPCs_.insert(entry.epc);
    totalProcessed_++;
    
    if (now_ms - lastCleanupTime_ > DUPLICATE_WINDOW_MS * 2) {
        recentEPCs_.clear();
        lastCleanupTime_ = now_ms;
    }
}

bool EPCBuffer::isDuplicate(const std::string& epc, int64_t) {
    auto it = recentEPCs_.find(epc);
    return it != recentEPCs_.end();
}

std::vector<EPCEntry> EPCBuffer::flush(int task_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::vector<EPCEntry> result;
    
    auto it = taskBuffers_.find(task_id);
    if (it != taskBuffers_.end()) {
        result.swap(it->second);
        if (it->second.empty()) {
            taskBuffers_.erase(it);
        }
    }
    
    return result;
}

std::vector<EPCEntry> EPCBuffer::flushAll() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::vector<EPCEntry> result;
    
    for (auto& pair : taskBuffers_) {
        result.insert(result.end(), pair.second.begin(), pair.second.end());
        pair.second.clear();
    }
    taskBuffers_.clear();
    
    return result;
}

size_t EPCBuffer::size(int task_id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = taskBuffers_.find(task_id);
    if (it != taskBuffers_.end()) {
        return it->second.size();
    }
    return 0;
}

size_t EPCBuffer::totalSize() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    size_t total = 0;
    for (const auto& pair : taskBuffers_) {
        total += pair.second.size();
    }
    return total;
}

bool EPCBuffer::empty() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return taskBuffers_.empty();
}

void EPCBuffer::clear(int task_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = taskBuffers_.find(task_id);
    if (it != taskBuffers_.end()) {
        taskBuffers_.erase(it);
    }
}

void EPCBuffer::clearAll() {
    std::lock_guard<std::mutex> lock(mutex_);
    taskBuffers_.clear();
    recentEPCs_.clear();
}

EPCProcessor::EPCProcessor()
    : running_(false), intervalMs_(500) {}

EPCProcessor::~EPCProcessor() {
    stop();
}

void EPCProcessor::setProcessCallback(ProcessCallback callback) {
    callback_ = callback;
}

void EPCProcessor::start(int intervalMs) {
    if (running_) return;
    
    intervalMs_ = intervalMs;
    running_ = true;
    
    processorThread_ = std::thread([this]() { processingLoop(); });
    spdlog::info("EPC processor started with {}ms interval", intervalMs_);
}

void EPCProcessor::stop() {
    if (!running_) return;
    
    running_ = false;
    
    if (processorThread_.joinable()) {
        processorThread_.join();
    }
    
    spdlog::info("EPC processor stopped");
}

void EPCProcessor::processingLoop() {
    while (running_) {
        std::this_thread::sleep_for(std::chrono::milliseconds(intervalMs_));
        
        if (!running_) break;
        
        auto entries = buffer_.flushAll();
        
        if (!entries.empty() && callback_) {
            try {
                callback_(entries);
            } catch (const std::exception& e) {
                spdlog::error("EPC processing callback failed: {}", e.what());
            }
        }
    }
}

std::vector<std::string> batchQueryAssetsByEPCs(const std::vector<std::string>& epcs) {
    if (epcs.empty()) return {};
    
    auto conn = DBPool::instance().acquire();
    pqxx::work W(*conn);
    
    std::stringstream ss;
    ss << "SELECT rfid_epc FROM assets WHERE rfid_epc IN (";
    for (size_t i = 0; i < epcs.size(); ++i) {
        if (i > 0) ss << ",";
        ss << W.quote(epcs[i]);
    }
    ss << ") AND status != 'SCRAPPED'";
    
    pqxx::result R = W.exec(ss.str());
    
    std::vector<std::string> foundEPCs;
    for (const auto& row : R) {
        foundEPCs.push_back(row[0].as<std::string>());
    }
    
    DBPool::instance().release(conn);
    
    return foundEPCs;
}

} // namespace rfid