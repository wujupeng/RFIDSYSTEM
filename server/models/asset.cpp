#include "asset.h"

bool AssetStatusMachine::canTransition(AssetStatus from, AssetStatus to) {
    switch (from) {
        case AssetStatus::IN_STOCK:
            return to == AssetStatus::IN_USE || to == AssetStatus::REPAIR;
        case AssetStatus::IN_USE:
            return to == AssetStatus::IN_STOCK || to == AssetStatus::REPAIR || to == AssetStatus::SCRAPPED;
        case AssetStatus::REPAIR:
            return to == AssetStatus::IN_STOCK || to == AssetStatus::SCRAPPED;
        case AssetStatus::SCRAPPED:
            return false;
        default:
            return false;
    }
}

std::string AssetStatusMachine::statusToString(AssetStatus status) {
    switch (status) {
        case AssetStatus::IN_STOCK: return "IN_STOCK";
        case AssetStatus::IN_USE: return "IN_USE";
        case AssetStatus::REPAIR: return "REPAIR";
        case AssetStatus::SCRAPPED: return "SCRAPPED";
        default: return "UNKNOWN";
    }
}

AssetStatus AssetStatusMachine::stringToStatus(const std::string& statusStr) {
    if (statusStr == "IN_STOCK") return AssetStatus::IN_STOCK;
    if (statusStr == "IN_USE") return AssetStatus::IN_USE;
    if (statusStr == "REPAIR") return AssetStatus::REPAIR;
    if (statusStr == "SCRAPPED") return AssetStatus::SCRAPPED;
    return AssetStatus::IN_STOCK;
}