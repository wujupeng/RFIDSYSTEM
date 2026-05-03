#pragma once
#include <string>
#include <vector>

struct InventoryResult {
    std::vector<std::string> found;
    std::vector<std::string> missing;
    std::vector<std::string> extra;
};

struct InventoryTask {
    int id;
    std::string task_name;
    std::string status;
    int scanned_count;
    int found_count;
    int missing_count;
    int extra_count;
    std::string location;
    std::string operator_name;
    int64_t created_at;
    int64_t completed_at;
};

struct ScanResult {
    std::string epc;
    int asset_id;
    std::string asset_code;
    std::string asset_name;
    std::string status;
};