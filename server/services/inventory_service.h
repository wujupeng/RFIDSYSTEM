#pragma once
#include "../models/inventory.h"
#include <memory>
#include <string>
#include <vector>

class InventoryService {
public:
    struct ScanResultDetail {
        std::string epc;
        int asset_id;
        std::string asset_code;
        std::string asset_name;
        std::string status;
    };

    static InventoryService& instance();

    int startTask(const std::string& taskName, const std::string& location, const std::string& operatorName);
    bool updateTask(int taskId, int scannedCount, int foundCount, int missingCount, int extraCount);
    bool completeTask(int taskId);
    InventoryTask getTask(int taskId);
    
    InventoryResult scanEPCs(const std::vector<std::string>& epcs, int taskId = 0);
    std::vector<ScanResultDetail> scanEPCsWithDetails(const std::vector<std::string>& epcs, int taskId = 0);

private:
    InventoryService() = default;
    InventoryService(const InventoryService&) = delete;
    InventoryService& operator=(const InventoryService&) = delete;
};