#include "reader.h"
#include "../core/logger.h"
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <sstream>

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
    
    fd_ = open(port_.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd_ < 0) {
        spdlog::error("Serial reader {}: failed to open port {} - {}", readerId_, port_, strerror(errno));
        return false;
    }
    
    struct termios options;
    tcgetattr(fd_, &options);
    
    speed_t baudSpeed;
    switch (baudRate_) {
        case 9600: baudSpeed = B9600; break;
        case 19200: baudSpeed = B19200; break;
        case 38400: baudSpeed = B38400; break;
        case 57600: baudSpeed = B57600; break;
        case 115200: baudSpeed = B115200; break;
        case 230400: baudSpeed = B230400; break;
        default: baudSpeed = B115200;
    }
    
    cfsetispeed(&options, baudSpeed);
    cfsetospeed(&options, baudSpeed);
    
    options.c_cflag |= (CLOCAL | CREAD);
    options.c_cflag &= ~PARENB;
    options.c_cflag &= ~CSTOPB;
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    options.c_oflag &= ~OPOST;
    options.c_cc[VMIN] = 1;
    options.c_cc[VTIME] = 0;
    
    tcsetattr(fd_, TCSANOW, &options);
    tcflush(fd_, TCIFLUSH);
    
    connected_ = true;
    spdlog::info("Serial reader {} connected to {}", readerId_, port_);
    return true;
}

void SerialReader::disconnect() {
    if (fd_ >= 0) {
        close(fd_);
        fd_ = -1;
    }
    connected_ = false;
    spdlog::info("Serial reader {} disconnected", readerId_);
}

bool SerialReader::isConnected() const {
    return connected_ && fd_ >= 0;
}

void SerialReader::startScanning() {
    if (!connected_) {
        spdlog::warn("Serial reader {}: cannot start scanning - not connected", readerId_);
        return;
    }
    
    scanning_ = true;
    spdlog::info("Serial reader {} started scanning", readerId_);
    
    if (readThread_.joinable()) {
        readThread_.join();
    }
    
    readThread_ = std::thread([this]() {
        readLoop();
    });
}

void SerialReader::stopScanning() {
    scanning_ = false;
    if (readThread_.joinable()) {
        readThread_.join();
    }
    spdlog::info("Serial reader {} stopped scanning", readerId_);
}

bool SerialReader::isScanning() const {
    return scanning_;
}

void SerialReader::readLoop() {
    fd_set readSet;
    struct timeval timeout;
    std::string buffer;
    
    while (scanning_ && connected_) {
        FD_ZERO(&readSet);
        FD_SET(fd_, &readSet);
        
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        
        int ret = select(fd_ + 1, &readSet, nullptr, nullptr, &timeout);
        
        if (ret < 0) {
            spdlog::error("Serial reader {}: select error - {}", readerId_, strerror(errno));
            onConnectionLost();
            break;
        }
        
        if (ret == 0) {
            continue;
        }
        
        if (FD_ISSET(fd_, &readSet)) {
            char buf[256];
            ssize_t n = read(fd_, buf, sizeof(buf) - 1);
            
            if (n < 0) {
                spdlog::error("Serial reader {}: read error - {}", readerId_, strerror(errno));
                onConnectionLost();
                break;
            }
            
            if (n == 0) {
                spdlog::warn("Serial reader {}: connection closed by peer", readerId_);
                onConnectionLost();
                break;
            }
            
            buf[n] = '\0';
            buffer += buf;
            
            size_t newlinePos;
            while ((newlinePos = buffer.find('\n')) != std::string::npos) {
                std::string line = buffer.substr(0, newlinePos);
                buffer = buffer.substr(newlinePos + 1);
                parseLine(line);
            }
        }
    }
}

void SerialReader::parseLine(const std::string& line) {
    std::string trimmed = line;
    trimmed.erase(trimmed.find_last_not_of("\r\n") + 1);
    
    if (trimmed.empty()) return;
    
    spdlog::debug("Serial reader {}: received line - {}", readerId_, trimmed);
    
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream iss(trimmed);
    
    while (std::getline(iss, token, ',')) {
        tokens.push_back(token);
    }
    
    if (tokens.size() >= 2 && tokens[0] == "EPC") {
        std::string epc = tokens[1];
        int rssi = (tokens.size() > 2) ? std::stoi(tokens[2]) : -50;
        
        if (rssi >= rssiThreshold_) {
            int64_t now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count();
            
            EPCEvent event;
            event.epc = epc;
            event.reader_id = readerId_;
            event.task_id = 0;
            event.rssi = rssi;
            event.timestamp_ms = now_ms;
            
            if (callback_) {
                callback_({event});
            }
            
            spdlog::debug("Serial reader {}: scanned EPC {} (RSSI: {})", readerId_, epc, rssi);
        }
    }
}

TcpReader::TcpReader(const std::string& reader_id, const std::string& host, int port)
    : Reader(reader_id, ReaderType::TCP), host_(host), port_(port), connected_(false), scanning_(false), sock_(-1) {}

TcpReader::~TcpReader() {
    stopScanning();
    disconnect();
}

bool TcpReader::connect() {
    spdlog::info("TCP reader {}: connecting to {}:{}", readerId_, host_, port_);
    
    sock_ = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_ < 0) {
        spdlog::error("TCP reader {}: failed to create socket - {}", readerId_, strerror(errno));
        return false;
    }
    
    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port_);
    
    if (inet_pton(AF_INET, host_.c_str(), &serverAddr.sin_addr) <= 0) {
        spdlog::error("TCP reader {}: invalid address - {}", readerId_, host_);
        close(sock_);
        sock_ = -1;
        return false;
    }
    
    if (::connect(sock_, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        spdlog::error("TCP reader {}: connect failed - {}", readerId_, strerror(errno));
        close(sock_);
        sock_ = -1;
        return false;
    }
    
    int flags = fcntl(sock_, F_GETFL, 0);
    fcntl(sock_, F_SETFL, flags | O_NONBLOCK);
    
    connected_ = true;
    spdlog::info("TCP reader {} connected to {}:{}", readerId_, host_, port_);
    return true;
}

void TcpReader::disconnect() {
    if (sock_ >= 0) {
        close(sock_);
        sock_ = -1;
    }
    connected_ = false;
    spdlog::info("TCP reader {} disconnected", readerId_);
}

bool TcpReader::isConnected() const {
    return connected_ && sock_ >= 0;
}

void TcpReader::startScanning() {
    if (!connected_) {
        spdlog::warn("TCP reader {}: cannot start scanning - not connected", readerId_);
        return;
    }
    
    scanning_ = true;
    spdlog::info("TCP reader {} started scanning", readerId_);
    
    if (readThread_.joinable()) {
        readThread_.join();
    }
    
    readThread_ = std::thread([this]() {
        readLoop();
    });
}

void TcpReader::stopScanning() {
    scanning_ = false;
    if (readThread_.joinable()) {
        readThread_.join();
    }
    spdlog::info("TCP reader {} stopped scanning", readerId_);
}

bool TcpReader::isScanning() const {
    return scanning_;
}

void TcpReader::readLoop() {
    fd_set readSet;
    struct timeval timeout;
    std::string buffer;
    
    while (scanning_ && connected_) {
        FD_ZERO(&readSet);
        FD_SET(sock_, &readSet);
        
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        
        int ret = select(sock_ + 1, &readSet, nullptr, nullptr, &timeout);
        
        if (ret < 0) {
            spdlog::error("TCP reader {}: select error - {}", readerId_, strerror(errno));
            onConnectionLost();
            break;
        }
        
        if (ret == 0) {
            continue;
        }
        
        if (FD_ISSET(sock_, &readSet)) {
            char buf[512];
            ssize_t n = read(sock_, buf, sizeof(buf) - 1);
            
            if (n < 0) {
                if (errno != EWOULDBLOCK && errno != EAGAIN) {
                    spdlog::error("TCP reader {}: read error - {}", readerId_, strerror(errno));
                    onConnectionLost();
                    break;
                }
                continue;
            }
            
            if (n == 0) {
                spdlog::warn("TCP reader {}: connection closed by peer", readerId_);
                onConnectionLost();
                break;
            }
            
            buf[n] = '\0';
            buffer += buf;
            
            size_t delimiterPos;
            while ((delimiterPos = buffer.find('\n')) != std::string::npos) {
                std::string line = buffer.substr(0, delimiterPos);
                buffer = buffer.substr(delimiterPos + 1);
                parseLine(line);
            }
        }
    }
}

void TcpReader::parseLine(const std::string& line) {
    std::string trimmed = line;
    trimmed.erase(trimmed.find_last_not_of("\r\n") + 1);
    
    if (trimmed.empty()) return;
    
    spdlog::debug("TCP reader {}: received line - {}", readerId_, trimmed);
    
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream iss(trimmed);
    
    while (std::getline(iss, token, ',')) {
        tokens.push_back(token);
    }
    
    if (tokens.size() >= 2 && tokens[0] == "EPC") {
        std::string epc = tokens[1];
        int rssi = (tokens.size() > 2) ? std::stoi(tokens[2]) : -50;
        
        if (rssi >= rssiThreshold_) {
            int64_t now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count();
            
            EPCEvent event;
            event.epc = epc;
            event.reader_id = readerId_;
            event.task_id = 0;
            event.rssi = rssi;
            event.timestamp_ms = now_ms;
            
            if (callback_) {
                callback_({event});
            }
            
            spdlog::debug("TCP reader {}: scanned EPC {} (RSSI: {})", readerId_, epc, rssi);
        }
    }
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
    
    if (scanThread_.joinable()) {
        scanThread_.join();
    }
    
    scanThread_ = std::thread([this]() {
        simulateLoop();
    });
}

void SimulatorReader::stopScanning() {
    scanning_ = false;
    if (scanThread_.joinable()) {
        scanThread_.join();
    }
    spdlog::info("Simulator reader {} stopped scanning", readerId_);
}

bool SimulatorReader::isScanning() const {
    return scanning_;
}

void SimulatorReader::simulateLoop() {
    while (scanning_ && connected_) {
        if (!epcList_.empty()) {
            size_t count = std::min((size_t)5, epcList_.size());
            std::vector<std::string> batch;
            
            for (size_t i = 0; i < count; ++i) {
                size_t idx = rand() % epcList_.size();
                batch.push_back(epcList_[idx]);
            }
            
            simulateScanBatch(batch);
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(500 + rand() % 500));
    }
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