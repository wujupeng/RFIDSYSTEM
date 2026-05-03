#include "reader.h"
#include "../core/logger.h"

namespace rfid {

Reader::Reader(const std::string& reader_id, ReaderType type)
    : readerId_(reader_id), type_(type) {}

void Reader::triggerReconnect() {
    std::lock_guard<std::mutex> lock(reconnectMutex_);
    if (reconnecting_ || !autoReconnectEnabled_) {
        return;
    }
    
    reconnecting_ = true;
    if (reconnectThread_.joinable()) {
        reconnectThread_.join();
    }
    
    reconnectThread_ = std::thread([this]() {
        reconnectLoop();
    });
}

void Reader::reconnectLoop() {
    int attempts = 0;
    
    while (attempts < MAX_RECONNECT_ATTEMPTS) {
        spdlog::warn("Reader {}: reconnect attempt {}/{}", readerId_, attempts + 1, MAX_RECONNECT_ATTEMPTS);
        
        if (connect()) {
            spdlog::info("Reader {}: reconnected successfully", readerId_);
            onConnectionRestored();
            reconnecting_ = false;
            return;
        }
        
        attempts++;
        std::this_thread::sleep_for(RECONNECT_DELAY);
    }
    
    spdlog::error("Reader {}: failed to reconnect after {} attempts", readerId_, MAX_RECONNECT_ATTEMPTS);
    reconnecting_ = false;
}

void Reader::onConnectionLost() {
    spdlog::warn("Reader {}: connection lost", readerId_);
    if (connStatusCallback_) {
        connStatusCallback_(readerId_, false);
    }
    if (autoReconnectEnabled_) {
        triggerReconnect();
    }
}

void Reader::onConnectionRestored() {
    spdlog::info("Reader {}: connection restored", readerId_);
    if (connStatusCallback_) {
        connStatusCallback_(readerId_, true);
    }
}

SerialReader::SerialReader(const std::string& reader_id, const std::string& port, int baudRate)
    : Reader(reader_id, ReaderType::SERIAL), port_(port), baudRate_(baudRate), connected_(false), scanning_(false), fd_(-1) {}

SerialReader::~SerialReader() {
    stopScanning();
    disconnect();
}

bool SerialReader::connect() {
    spdlog::info("Serial reader {}: connecting to port {}", readerId_, port_);
    connected_ = true;
    spdlog::info("Serial reader {} connected", readerId_);
    return true;
}

void SerialReader::disconnect() {
    connected_ = false;
    spdlog::info("Serial reader {} disconnected", readerId_);
}

bool SerialReader::isConnected() const {
    return connected_;
}

void SerialReader::startScanning() {
    scanning_ = true;
    spdlog::info("Serial reader {} started scanning", readerId_);
}

void SerialReader::stopScanning() {
    scanning_ = false;
    spdlog::info("Serial reader {} stopped scanning", readerId_);
}

bool SerialReader::isScanning() const {
    return scanning_;
}

TcpReader::TcpReader(const std::string& reader_id, const std::string& host, int port)
    : Reader(reader_id, ReaderType::TCP), host_(host), port_(port), connected_(false), scanning_(false), sock_(-1) {}

TcpReader::~TcpReader() {
    stopScanning();
    disconnect();
}

bool TcpReader::connect() {
    spdlog::info("TCP reader {}: connecting to {}:{}", readerId_, host_, port_);
    connected_ = true;
    spdlog::info("TCP reader {} connected", readerId_);
    return true;
}

void TcpReader::disconnect() {
    connected_ = false;
    spdlog::info("TCP reader {} disconnected", readerId_);
}

bool TcpReader::isConnected() const {
    return connected_;
}

void TcpReader::startScanning() {
    scanning_ = true;
    spdlog::info("TCP reader {} started scanning", readerId_);
}

void TcpReader::stopScanning() {
    scanning_ = false;
    spdlog::info("TCP reader {} stopped scanning", readerId_);
}

bool TcpReader::isScanning() const {
    return scanning_;
}

SimulatorReader::SimulatorReader(const std::string& reader_id)
    : Reader(reader_id, ReaderType::SIMULATOR), connected_(false), scanning_(false) {}

SimulatorReader::~SimulatorReader() {
    stopScanning();
    disconnect();
}

bool SimulatorReader::connect() {
    connected_ = true;
    spdlog::info("Simulator reader {} connected", readerId_);
    return true;
}

void SimulatorReader::disconnect() {
    connected_ = false;
    spdlog::info("Simulator reader {} disconnected", readerId_);
}

bool SimulatorReader::isConnected() const {
    return connected_;
}

void SimulatorReader::startScanning() {
    scanning_ = true;
    spdlog::info("Simulator reader {} started scanning", readerId_);
}

void SimulatorReader::stopScanning() {
    scanning_ = false;
    spdlog::info("Simulator reader {} stopped scanning", readerId_);
}

bool SimulatorReader::isScanning() const {
    return scanning_;
}

void SimulatorReader::simulateScan(const std::string& epc, int rssi) {
    if (!scanning_ || !connected_) return;

    int64_t now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();

    if (rssi < rssiThreshold_) {
        spdlog::debug("Simulator {}: filtered EPC {} due to low RSSI {}", readerId_, epc, rssi);
        return;
    }

    std::vector<EPCEvent> batch;
    EPCEvent event;
    event.epc = epc;
    event.reader_id = readerId_;
    event.task_id = 0;
    event.rssi = rssi;
    event.timestamp_ms = now_ms;
    batch.push_back(event);

    if (callback_) {
        callback_(batch);
    }

    spdlog::debug("Simulator {}: scanned EPC {} (RSSI: {})", readerId_, epc, rssi);
}

void SimulatorReader::simulateScanBatch(const std::vector<std::string>& epcs) {
    if (!scanning_ || !connected_) return;

    int64_t now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();

    std::vector<EPCEvent> batch;

    for (const auto& epc : epcs) {
        int rssi = -50 + (rand() % 20);
        
        if (rssi < rssiThreshold_) continue;

        EPCEvent event;
        event.epc = epc;
        event.reader_id = readerId_;
        event.task_id = 0;
        event.rssi = rssi;
        event.timestamp_ms = now_ms;
        batch.push_back(event);
    }

    if (!batch.empty() && callback_) {
        callback_(batch);
    }

    spdlog::debug("Simulator {}: scanned batch of {} EPCs", readerId_, batch.size());
}

void SimulatorReader::setEPCList(const std::vector<std::string>& epcs) {
    epcList_ = epcs;
}

ReaderManager& ReaderManager::instance() {
    static ReaderManager instance;
    return instance;
}

void ReaderManager::addReader(std::shared_ptr<Reader> reader) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!reader) return;
    
    auto readerId = reader->getReaderId();
    if (readers_.count(readerId)) {
        spdlog::warn("Reader {} already exists, replacing", readerId);
    }
    
    readers_[readerId] = reader;
    spdlog::info("Reader {} added (type: {})", readerId, static_cast<int>(reader->getReaderType()));
}

void ReaderManager::removeReader(const std::string& readerId) {
    std::lock_guard<std::mutex> lock(mutex_);
    readers_.erase(readerId);
    spdlog::info("Reader {} removed", readerId);
}

void ReaderManager::startAll() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    for (auto& pair : readers_) {
        auto& reader = pair.second;
        if (!reader->isConnected()) {
            reader->connect();
        }
        reader->startScanning();
    }
    
    spdlog::info("All {} readers started", readers_.size());
}

void ReaderManager::stopAll() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    for (auto& pair : readers_) {
        auto& reader = pair.second;
        reader->stopScanning();
        reader->disconnect();
    }
    
    spdlog::info("All readers stopped");
}

std::shared_ptr<Reader> ReaderManager::getReader(const std::string& readerId) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = readers_.find(readerId);
    if (it != readers_.end()) {
        return it->second;
    }
    return nullptr;
}

std::vector<std::string> ReaderManager::getReaderIds() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<std::string> ids;
    for (const auto& pair : readers_) {
        ids.push_back(pair.first);
    }
    return ids;
}

size_t ReaderManager::getReaderCount() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return readers_.size();
}

size_t ReaderManager::getConnectedReaderCount() const {
    std::lock_guard<std::mutex> lock(mutex_);
    size_t count = 0;
    for (const auto& pair : readers_) {
        if (pair.second->isConnected()) {
            count++;
        }
    }
    return count;
}

std::vector<EPCData> ReaderManager::aggregateEPCs(const std::vector<EPCEvent>& events) {
    std::unordered_map<std::string, EPCData> epcMap;
    
    for (const auto& event : events) {
        auto key = std::to_string(event.task_id) + ":" + event.epc;
        auto it = epcMap.find(key);
        
        if (it == epcMap.end()) {
            EPCData data;
            data.epc = event.epc;
            data.task_id = event.task_id;
            data.best_reader_id = event.reader_id;
            data.best_rssi = event.rssi;
            data.latest_timestamp = event.timestamp_ms;
            data.reader_ids.push_back(event.reader_id);
            epcMap[key] = data;
        } else {
            it->second.reader_ids.push_back(event.reader_id);
            
            if (event.rssi > it->second.best_rssi) {
                it->second.best_rssi = event.rssi;
                it->second.best_reader_id = event.reader_id;
            }
            
            if (event.timestamp_ms > it->second.latest_timestamp) {
                it->second.latest_timestamp = event.timestamp_ms;
            }
        }
    }
    
    std::vector<EPCData> result;
    for (auto& pair : epcMap) {
        result.push_back(pair.second);
    }
    
    return result;
}

std::shared_ptr<Reader> createReader(const std::string& reader_id, ReaderType type, const std::string& connectionString) {
    switch (type) {
        case ReaderType::SERIAL:
            return std::make_shared<SerialReader>(reader_id, connectionString);
        case ReaderType::TCP: {
            size_t colonPos = connectionString.find(':');
            std::string host = connectionString.substr(0, colonPos);
            int port = std::stoi(connectionString.substr(colonPos + 1));
            return std::make_shared<TcpReader>(reader_id, host, port);
        }
        case ReaderType::SIMULATOR:
            return std::make_shared<SimulatorReader>(reader_id);
        default:
            return nullptr;
    }
}

} // namespace rfid