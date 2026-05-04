#pragma once
#include <string>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <mutex>
#include <chrono>
#include <functional>
#include <thread>
#include <queue>

namespace rfid {

struct EPCEntry {
    std::string epc;
    int task_id;
    int64_t timestamp_ms;
    int rssi;
    std::string reader_id;
};

class EPCBuffer {
public:
    static const size_t MAX_BUFFER_SIZE = 10000;
    static const int64_t DUPLICATE_WINDOW_MS = 1000;

    EPCBuffer();

    void push(const std::string& epc, int task_id = 0, int rssi = -60, const std::string& reader_id = "");
    void push(const EPCEntry& entry);
    
    std::vector<EPCEntry> flush(int task_id = 0);
    std::vector<EPCEntry> flushAll();

    size_t size(int task_id = 0) const;
    size_t totalSize() const;
    bool empty() const;
    void clear(int task_id = 0);
    void clearAll();

    size_t getTotalProcessed() const { return totalProcessed_; }
    size_t getTotalDuplicates() const { return totalDuplicates_; }

private:
    bool isDuplicate(const std::string& epc, int64_t now_ms);

    std::unordered_map<int, std::vector<EPCEntry>> taskBuffers_;
    std::unordered_set<std::string> recentEPCs_;
    
    mutable std::mutex mutex_;
    
    int64_t lastCleanupTime_;
    
    size_t totalProcessed_;
    size_t totalDuplicates_;
};

class EPCProcessor {
public:
    using ProcessCallback = std::function<void(const std::vector<EPCEntry>& entries)>;

    EPCProcessor();
    ~EPCProcessor();

    void setProcessCallback(ProcessCallback callback);
    void start(int intervalMs = 500);
    void stop();

    EPCBuffer& buffer() { return buffer_; }

private:
    void processingLoop();

    EPCBuffer buffer_;
    std::thread processorThread_;
    bool running_;
    int intervalMs_;
    ProcessCallback callback_;
};

std::vector<std::string> batchQueryAssetsByEPCs(const std::vector<std::string>& epcs);

} // namespace rfid