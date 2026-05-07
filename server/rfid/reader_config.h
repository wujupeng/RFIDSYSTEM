#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <mutex>
#include "reader.h"

namespace rfid {

struct ReaderConfig {
    std::string reader_id;
    ReaderType type;
    std::string connection_string;
    int baud_rate = 115200;
    int rssi_threshold = DEFAULT_RSSI_THRESHOLD;
    bool auto_reconnect = true;
    bool enabled = true;
};

class ReaderConfigManager {
public:
    static ReaderConfigManager& instance();

    bool loadConfig(const std::string& filePath);
    bool saveConfig(const std::string& filePath) const;

    void addConfig(const ReaderConfig& config);
    void removeConfig(const std::string& readerId);
    ReaderConfig getConfig(const std::string& readerId) const;
    std::vector<ReaderConfig> getAllConfigs() const;
    size_t getConfigCount() const;

    std::shared_ptr<Reader> createReaderFromConfig(const ReaderConfig& config);
    void createAllReaders();

private:
    ReaderConfigManager() = default;
    ReaderConfigManager(const ReaderConfigManager&) = delete;
    ReaderConfigManager& operator=(const ReaderConfigManager&) = delete;

    std::unordered_map<std::string, ReaderConfig> configs_;
    mutable std::mutex mutex_;
};

} // namespace rfid