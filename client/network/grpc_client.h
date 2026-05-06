#pragma once
#include <string>
#include <vector>
#include <memory>

struct Asset {
    int id;
    std::string asset_code;
    std::string name;
    std::string type;
    std::string rfid_epc;
    std::string location;
    std::string status;
};

struct ScanResult {
    std::string epc;
    int asset_id;
    std::string asset_code;
    std::string asset_name;
    std::string status;
};

struct BatchScanResult {
    std::vector<ScanResult> found;
    std::vector<std::string> missing;
    std::vector<std::string> extra;
    int total_scanned;
    int total_found;
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
};

struct DecisionView {
    int asset_id;
    std::string asset_name;
    std::string location;
    std::string action;
    std::string risk_level;
    std::string reason;
    std::string timestamp;
    bool is_handled;
    bool is_executed;
    bool is_ignored;
};

struct DecisionsResponse {
    std::vector<DecisionView> decisions;
    int total_count;
    double adoption_rate;
};

struct AssetStatistics {
    int64_t total_assets;
    int64_t in_stock_count;
    int64_t in_use_count;
    int64_t repair_count;
    int64_t scrapped_count;
};

class GrpcClient {
public:
    struct CreateAssetParams {
        std::string name;
        std::string type;
        std::string asset_code;
        std::string rfid_epc;
        std::string location;
        std::string operator_name;
    };

    struct ListAssetsParams {
        int page;
        int page_size;
        std::string status_filter;
    };

    explicit GrpcClient(const std::string& server_address);
    ~GrpcClient();

    int createAsset(const CreateAssetParams& params);
    Asset getAsset(int id);
    bool updateAssetStatus(int id, const std::string& newStatus, const std::string& operatorName);
    std::vector<Asset> listAssets(const ListAssetsParams& params);

    int startInventoryTask(const std::string& taskName, const std::string& location, const std::string& operatorName);
    BatchScanResult batchScanEPC(const std::vector<std::string>& epcs, int taskId = 0);
    InventoryTask getInventoryTask(int taskId);
    bool completeInventoryTask(int taskId);

    DecisionsResponse getRecentDecisions(int limit = 100, const std::string& userFilter = "");
    bool reportDecision(int assetId, bool executed, bool ignored, const std::string& userName = "operator");
    AssetStatistics getAssetStatistics();

private:
    class Impl;
    std::unique_ptr<Impl> pImpl_;
};
