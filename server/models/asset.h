#pragma once
#include <string>
#include <stdexcept>

enum class AssetStatus {
    IN_STOCK,
    IN_USE,
    REPAIR,
    SCRAPPED
};

struct Asset {
    int id;
    std::string asset_code;
    std::string name;
    std::string type;
    std::string rfid_epc;
    std::string location;
    AssetStatus status;
    int64_t created_at;
    int64_t updated_at;
};

class AssetStatusMachine {
public:
    static bool canTransition(AssetStatus from, AssetStatus to);
    static std::string statusToString(AssetStatus status);
    static AssetStatus stringToStatus(const std::string& statusStr);

    static bool isValidStatus(const std::string& statusStr) {
        return statusStr == "IN_STOCK" || statusStr == "IN_USE" ||
               statusStr == "REPAIR" || statusStr == "SCRAPPED";
    }
};

class AssetException : public std::runtime_error {
public:
    explicit AssetException(const std::string& msg) : std::runtime_error(msg) {}
};

class InvalidStatusTransitionException : public AssetException {
public:
    InvalidStatusTransitionException(const std::string& from, const std::string& to)
        : AssetException("Invalid status transition from " + from + " to " + to) {}
};

class AssetNotFoundException : public AssetException {
public:
    explicit AssetNotFoundException(int id)
        : AssetException("Asset not found with id: " + std::to_string(id)) {}
};