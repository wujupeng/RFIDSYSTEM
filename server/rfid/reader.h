#pragma once
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <unordered_map>
#include <mutex>
#include <chrono>
#include <set>
#include <thread>

namespace rfid {

enum class ReaderType {
    SERIAL,
    TCP,
    SIMULATOR
};

constexpr int DEFAULT_RSSI_THRESHOLD = -70;
constexpr int64_t DUPLICATE_FILTER_WINDOW_MS = 1000;
constexpr size_t MAX_BUFFER_SIZE = 10000;
constexpr int MAX_RECONNECT_ATTEMPTS = 5;
constexpr std::chrono::seconds RECONNECT_DELAY = std::chrono::seconds(3);

struct EPCEvent {
    std::string epc;
    std::string reader_id;
    int task_id;
    int rssi;
    int64_t timestamp_ms;
};

struct EPCData {
    std::string epc;
    int task_id;
    std::string best_reader_id;
    int best_rssi;
    int64_t latest_timestamp;
    std::vector<std::string> reader_ids;
};

class Reader {
public:
    using ScanCallback = std::function<void(const std::vector<EPCEvent>& batch)>;
    using ConnectionStatusCallback = std::function<void(const std::string& readerId, bool connected)>;

    Reader(const std::string& reader_id, ReaderType type);
    virtual ~Reader() = default;

    virtual bool connect() = 0;
    virtual void disconnect() = 0;
    virtual bool isConnected() const = 0;

    virtual void startScanning() = 0;
    virtual void stopScanning() = 0;
    virtual bool isScanning() const = 0;

    std::string getReaderId() const { return readerId_; }
    ReaderType getReaderType() const { return type_; }

    void setScanCallback(ScanCallback callback) {
        callback_ = callback;
    }

    void setConnectionStatusCallback(ConnectionStatusCallback callback) {
        connStatusCallback_ = callback;
    }

    void setRSSIThreshold(int threshold) {
        rssiThreshold_ = threshold;
    }

    int getRSSIThreshold() const { return rssiThreshold_; }
    
    void enableAutoReconnect(bool enable) { autoReconnectEnabled_ = enable; }
    bool isAutoReconnectEnabled() const { return autoReconnectEnabled_; }

protected:
    void triggerReconnect();
    void onConnectionLost();
    void onConnectionRestored();

    ScanCallback callback_;
    ConnectionStatusCallback connStatusCallback_;
    std::string readerId_;
    ReaderType type_;
    int rssiThreshold_ = DEFAULT_RSSI_THRESHOLD;
    bool autoReconnectEnabled_ = true;
    
private:
    void reconnectLoop();
    
    std::thread reconnectThread_;
    bool reconnecting_ = false;
    std::mutex reconnectMutex_;
};

class SerialReader : public Reader {
public:
    SerialReader(const std::string& reader_id, const std::string& port, int baudRate = 115200);
    ~SerialReader() override;

    bool connect() override;
    void disconnect() override;
    bool isConnected() const override;

    void startScanning() override;
    void stopScanning() override;
    bool isScanning() const override;

private:
    std::string port_;
    int baudRate_;
    bool connected_;
    bool scanning_;
    int fd_;
};

class TcpReader : public Reader {
public:
    TcpReader(const std::string& reader_id, const std::string& host, int port);
    ~TcpReader() override;

    bool connect() override;
    void disconnect() override;
    bool isConnected() const override;

    void startScanning() override;
    void stopScanning() override;
    bool isScanning() const override;

private:
    std::string host_;
    int port_;
    bool connected_;
    bool scanning_;
    int sock_;
};

class SimulatorReader : public Reader {
public:
    SimulatorReader(const std::string& reader_id);
    ~SimulatorReader() override;

    bool connect() override;
    void disconnect() override;
    bool isConnected() const override;

    void startScanning() override;
    void stopScanning() override;
    bool isScanning() const override;

    void simulateScan(const std::string& epc, int rssi = -50);
    void simulateScanBatch(const std::vector<std::string>& epcs);
    void setEPCList(const std::vector<std::string>& epcs);

private:
    bool connected_;
    bool scanning_;
    std::vector<std::string> epcList_;
};

class ReaderManager {
public:
    using DataCallback = std::function<void(const std::vector<EPCData>& aggregated)>;

    static ReaderManager& instance();

    void addReader(std::shared_ptr<Reader> reader);
    void removeReader(const std::string& readerId);
    void startAll();
    void stopAll();

    std::shared_ptr<Reader> getReader(const std::string& readerId);
    std::vector<std::string> getReaderIds() const;
    size_t getReaderCount() const;
    size_t getConnectedReaderCount() const;

    void setDataCallback(DataCallback callback) {
        dataCallback_ = callback;
    }

    std::vector<EPCData> aggregateEPCs(const std::vector<EPCEvent>& events);

private:
    ReaderManager() = default;
    ReaderManager(const ReaderManager&) = delete;
    ReaderManager& operator=(const ReaderManager&) = delete;

    std::unordered_map<std::string, std::shared_ptr<Reader>> readers_;
    mutable std::mutex mutex_;
    DataCallback dataCallback_;
};

std::shared_ptr<Reader> createReader(const std::string& reader_id, ReaderType type, const std::string& connectionString);

} // namespace rfid