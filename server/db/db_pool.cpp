#include "db_pool.h"
#include "../core/logger.h"
#include <thread>

DBPool& DBPool::instance() {
    static DBPool instance;
    return instance;
}

DBPool::DBPool() : inUseCount_(0) {
    const char* maxConnEnv = std::getenv("RFID_DB_MAX_CONNECTIONS");
    size_t maxConnections = maxConnEnv ? std::stoul(maxConnEnv) : MAX_CONNECTIONS;

    spdlog::info("DB pool initializing with max connections: {}", maxConnections);

    for (size_t i = 0; i < maxConnections; ++i) {
        try {
            auto conn = createConnection();
            if (conn->is_open()) {
                pool_.push_back(conn);
                availableConnections_.push_back(conn);
                spdlog::debug("DB pool: created connection {}", i + 1);
            }
        } catch (const std::exception& e) {
            spdlog::error("DB pool: failed to create connection {} - {}", i + 1, e.what());
        }
    }

    spdlog::info("DB pool initialized: {} connections available", availableConnections_.size());
}

std::shared_ptr<pqxx::connection> DBPool::createConnection() {
    const char* dbname = std::getenv("RFID_DB_NAME") ? std::getenv("RFID_DB_NAME") : "rfid";
    const char* dbuser = std::getenv("RFID_DB_USER") ? std::getenv("RFID_DB_USER") : "postgres";
    const char* dbpass = std::getenv("RFID_DB_PASS") ? std::getenv("RFID_DB_PASS") : "123456";
    const char* dbhost = std::getenv("RFID_DB_HOST") ? std::getenv("RFID_DB_HOST") : "/var/run/postgresql";
    const char* dbport = std::getenv("RFID_DB_PORT") ? std::getenv("RFID_DB_PORT") : "5432";

    std::string hostParam;
    std::string hostStr(dbhost);
    if (!hostStr.empty() && hostStr[0] == '/') {
        hostParam = " host=" + hostStr;
    } else {
        hostParam = " hostaddr=" + hostStr + " port=" + std::string(dbport);
    }

    std::string connStr = "dbname=" + std::string(dbname) +
                          " user=" + std::string(dbuser) +
                          " password=" + std::string(dbpass) +
                          hostParam;

    return std::make_shared<pqxx::connection>(connStr);
}

bool DBPool::isValidConnection(const std::shared_ptr<pqxx::connection>& conn) {
    if (!conn || !conn->is_open()) {
        return false;
    }
    try {
        pqxx::nontransaction txn(*conn);
        txn.exec("SELECT 1");
        return true;
    } catch (const std::exception& e) {
        spdlog::warn("DB pool: connection validation failed - {}", e.what());
        return false;
    }
}

std::shared_ptr<pqxx::connection> DBPool::acquire() {
    return acquire_with_timeout(ACQUIRE_TIMEOUT);
}

std::shared_ptr<pqxx::connection> DBPool::acquire_with_timeout(std::chrono::milliseconds timeout) {
    std::unique_lock<std::mutex> lock(mutex_);

    // Check available connections
    for (auto it = availableConnections_.begin(); it != availableConnections_.end();) {
        if (!isValidConnection(*it)) {
            spdlog::warn("DB pool: removing invalid connection");
            it = availableConnections_.erase(it);
        } else {
            auto conn = *it;
            availableConnections_.erase(it);
            inUseCount_++;
            spdlog::debug("DB pool: acquired connection, in_use={}", inUseCount_);
            return conn;
        }
    }

    // Try to create new connection if pool is not full
    if (pool_.size() < MAX_CONNECTIONS) {
        try {
            auto conn = createConnection();
            if (conn->is_open()) {
                pool_.push_back(conn);
                inUseCount_++;
                spdlog::info("DB pool: created new connection, total={}", pool_.size());
                return conn;
            }
        } catch (const std::exception& e) {
            spdlog::error("DB pool: failed to create new connection - {}", e.what());
        }
    }

    spdlog::warn("DB pool: waiting for available connection, in_use={}, pool_size={}",
                 inUseCount_, pool_.size());

    auto waitResult = cv_.wait_for(lock, timeout, [this] {
        return !availableConnections_.empty();
    });

    if (!waitResult) {
        spdlog::error("DB pool: timeout waiting for connection");
        throw DBConnectionTimeout();
    }

    auto conn = availableConnections_.back();
    availableConnections_.pop_back();
    
    // Validate after wait - in modern libpqxx, recreate if invalid
    if (!isValidConnection(conn)) {
        spdlog::warn("DB pool: connection became invalid during wait, creating new one");
        try {
            conn = createConnection();
            if (conn->is_open()) {
                spdlog::info("DB pool: successfully created new connection");
            } else {
                throw std::runtime_error("Failed to create new connection");
            }
        } catch (const std::exception& e) {
            spdlog::error("DB pool: failed to create new connection - {}", e.what());
            throw DBConnectionError("Connection lost and reconnection failed");
        }
    }
    
    inUseCount_++;

    spdlog::debug("DB pool: acquired connection after wait, in_use={}", inUseCount_);
    return conn;
}

void DBPool::release(std::shared_ptr<pqxx::connection> conn) {
    std::unique_lock<std::mutex> lock(mutex_);

    if (inUseCount_ > 0) {
        inUseCount_--;
    }

    if (isValidConnection(conn)) {
        availableConnections_.push_back(conn);
        spdlog::debug("DB pool: released connection, in_use={}", inUseCount_);
        cv_.notify_one();
    } else {
        spdlog::warn("DB pool: releasing invalid connection");
        // Remove from pool
        auto it = std::find(pool_.begin(), pool_.end(), conn);
        if (it != pool_.end()) {
            pool_.erase(it);
        }
        // Try to create replacement connection
        try {
            auto newConn = createConnection();
            if (newConn->is_open()) {
                pool_.push_back(newConn);
                availableConnections_.push_back(newConn);
                spdlog::info("DB pool: replaced invalid connection");
            }
        } catch (const std::exception& e) {
            spdlog::error("DB pool: failed to replace invalid connection - {}", e.what());
        }
        cv_.notify_one();
    }
}

bool DBPool::isConnected() const {
    std::lock_guard<std::mutex> lock(mutex_);
    for (const auto& conn : pool_) {
        if (conn->is_open()) {
            return true;
        }
    }
    return false;
}

void DBPool::reconnectAll() {
    std::lock_guard<std::mutex> lock(mutex_);

    spdlog::info("DB pool: reconnecting all connections");

    for (size_t i = 0; i < pool_.size(); ++i) {
        int attempts = 0;
        while (attempts < MAX_RECONNECT_ATTEMPTS) {
            try {
                pool_[i] = createConnection();
                if (pool_[i]->is_open()) {
                    spdlog::info("DB pool: reconnected connection {}", i + 1);
                    break;
                }
            } catch (const std::exception& e) {
                spdlog::warn("DB pool: reconnect attempt {} failed - {}", attempts + 1, e.what());
            }
            attempts++;
            std::this_thread::sleep_for(RECONNECT_DELAY);
        }

        if (attempts >= MAX_RECONNECT_ATTEMPTS) {
            spdlog::error("DB pool: failed to reconnect connection {} after {} attempts", i + 1, MAX_RECONNECT_ATTEMPTS);
        }
    }

    availableConnections_.clear();
    for (const auto& conn : pool_) {
        if (conn->is_open()) {
            availableConnections_.push_back(conn);
        }
    }

    spdlog::info("DB pool: reconnect completed, {} connections available", availableConnections_.size());
}