#pragma once
#include <pqxx/pqxx>
#include <memory>
#include <vector>
#include <string>
#include <stdexcept>
#include <mutex>
#include <condition_variable>
#include <chrono>

class DBPool {
public:
    static DBPool& instance();

    std::shared_ptr<pqxx::connection> acquire();
    std::shared_ptr<pqxx::connection> acquire_with_timeout(std::chrono::milliseconds timeout);
    void release(std::shared_ptr<pqxx::connection> conn);

    size_t poolSize() const { return pool_.size(); }
    size_t availableCount() const { return availableConnections_.size(); }
    size_t inUseCount() const { return inUseCount_; }
    
    bool isConnected() const;
    void reconnectAll();

private:
    DBPool();
    ~DBPool() = default;

    DBPool(const DBPool&) = delete;
    DBPool& operator=(const DBPool&) = delete;

    std::shared_ptr<pqxx::connection> createConnection();
    bool isValidConnection(const std::shared_ptr<pqxx::connection>& conn);

    const size_t MAX_CONNECTIONS = 20;
    const std::chrono::milliseconds ACQUIRE_TIMEOUT = std::chrono::seconds(3);
    const int MAX_RECONNECT_ATTEMPTS = 5;
    const std::chrono::seconds RECONNECT_DELAY = std::chrono::seconds(2);

    std::vector<std::shared_ptr<pqxx::connection>> pool_;
    std::vector<std::shared_ptr<pqxx::connection>> availableConnections_;
    size_t inUseCount_;

    mutable std::mutex mutex_;
    std::condition_variable cv_;
};

class DBConnectionError : public std::runtime_error {
public:
    explicit DBConnectionError(const std::string& msg) : std::runtime_error(msg) {}
};

class DBConnectionTimeout : public DBConnectionError {
public:
    explicit DBConnectionTimeout() : DBConnectionError("DB connection acquire timeout") {}
};
