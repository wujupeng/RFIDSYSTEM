#include "reader_config.h"
#include "../core/logger.h"
#include <fstream>
#include <sstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace rfid {

ReaderConfigManager& ReaderConfigManager::instance() {
    static ReaderConfigManager instance;
    return instance;
}

bool ReaderConfigManager::loadConfig(const std::string& filePath) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::ifstream file(filePath);
    if (!file.is_open()) {
        spdlog::warn("Reader config file not found: {}", filePath);
        return false;
    }
    
    try {
        json configJson;
        file >> configJson;
        
        configs_.clear();
        
        if (configJson.contains("readers") && configJson["readers"].is_array()) {
            for (const auto& readerJson : configJson["readers"]) {
                ReaderConfig config;
                
                if (readerJson.contains("reader_id")) {
                    config.reader_id = readerJson["reader_id"].get<std::string>();
                }
                
                if (readerJson.contains("type")) {
                    std::string typeStr = readerJson["type"].get<std::string>();
                    if (typeStr == "SERIAL") {
                        config.type = ReaderType::SERIAL;
                    } else if (typeStr == "TCP") {
                        config.type = ReaderType::TCP;
                    } else if (typeStr == "SIMULATOR") {
                        config.type = ReaderType::SIMULATOR;
                    }
                }
                
                if (readerJson.contains("connection_string")) {
                    config.connection_string = readerJson["connection_string"].get<std::string>();
                }
                
                if (readerJson.contains("baud_rate")) {
                    config.baud_rate = readerJson["baud_rate"].get<int>();
                }
                
                if (readerJson.contains("rssi_threshold")) {
                    config.rssi_threshold = readerJson["rssi_threshold"].get<int>();
                }
                
                if (readerJson.contains("auto_reconnect")) {
                    config.auto_reconnect = readerJson["auto_reconnect"].get<bool>();
                }
                
                if (readerJson.contains("enabled")) {
                    config.enabled = readerJson["enabled"].get<bool>();
                }
                
                configs_[config.reader_id] = config;
            }
        }
        
        spdlog::info("Loaded {} reader configurations from {}", configs_.size(), filePath);
        return true;
        
    } catch (const std::exception& e) {
        spdlog::error("Failed to load reader config: {}", e.what());
        return false;
    }
}

bool ReaderConfigManager::saveConfig(const std::string& filePath) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::ofstream file(filePath);
    if (!file.is_open()) {
        spdlog::error("Failed to open config file for writing: {}", filePath);
        return false;
    }
    
    try {
        json configJson;
        json readersJson = json::array();
        
        for (const auto& pair : configs_) {
            const auto& config = pair.second;
            json readerJson;
            
            readerJson["reader_id"] = config.reader_id;
            
            switch (config.type) {
                case ReaderType::SERIAL:
                    readerJson["type"] = "SERIAL";
                    break;
                case ReaderType::TCP:
                    readerJson["type"] = "TCP";
                    break;
                case ReaderType::SIMULATOR:
                    readerJson["type"] = "SIMULATOR";
                    break;
            }
            
            readerJson["connection_string"] = config.connection_string;
            readerJson["baud_rate"] = config.baud_rate;
            readerJson["rssi_threshold"] = config.rssi_threshold;
            readerJson["auto_reconnect"] = config.auto_reconnect;
            readerJson["enabled"] = config.enabled;
            
            readersJson.push_back(readerJson);
        }
        
        configJson["readers"] = readersJson;
        
        file << configJson.dump(4);
        spdlog::info("Saved {} reader configurations to {}", configs_.size(), filePath);
        return true;
        
    } catch (const std::exception& e) {
        spdlog::error("Failed to save reader config: {}", e.what());
        return false;
    }
}

void ReaderConfigManager::addConfig(const ReaderConfig& config) {
    std::lock_guard<std::mutex> lock(mutex_);
    configs_[config.reader_id] = config;
    spdlog::info("Added reader config: {}", config.reader_id);
}

void ReaderConfigManager::removeConfig(const std::string& readerId) {
    std::lock_guard<std::mutex> lock(mutex_);
    configs_.erase(readerId);
    spdlog::info("Removed reader config: {}", readerId);
}

ReaderConfig ReaderConfigManager::getConfig(const std::string& readerId) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = configs_.find(readerId);
    if (it != configs_.end()) {
        return it->second;
    }
    return ReaderConfig();
}

std::vector<ReaderConfig> ReaderConfigManager::getAllConfigs() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<ReaderConfig> result;
    for (const auto& pair : configs_) {
        result.push_back(pair.second);
    }
    return result;
}

size_t ReaderConfigManager::getConfigCount() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return configs_.size();
}

std::shared_ptr<Reader> ReaderConfigManager::createReaderFromConfig(const ReaderConfig& config) {
    auto reader = rfid::createReader(config.reader_id, config.type, config.connection_string);
    if (reader) {
        reader->setRSSIThreshold(config.rssi_threshold);
        reader->enableAutoReconnect(config.auto_reconnect);
        spdlog::info("Created reader from config: {} (type: {})", config.reader_id, static_cast<int>(config.type));
    }
    return reader;
}

void ReaderConfigManager::createAllReaders() {
    auto configs = getAllConfigs();
    for (const auto& config : configs) {
        if (config.enabled) {
            auto reader = createReaderFromConfig(config);
            if (reader) {
                ReaderManager::instance().addReader(reader);
            }
        }
    }
    spdlog::info("Created {} readers from config", ReaderManager::instance().getReaderCount());
}

} // namespace rfid