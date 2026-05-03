#pragma once
#include "../models/asset.h"
#include <vector>
#include <string>
#include <memory>

class AssetService {
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

    static AssetService& instance();

    int createAsset(const CreateAssetParams& params);
    Asset getAsset(int id);
    bool updateAssetStatus(int id, const std::string& newStatus, const std::string& operatorName);
    std::vector<Asset> listAssets(const ListAssetsParams& params);
    int getTotalCount(const std::string& statusFilter = "");

private:
    AssetService() = default;
    AssetService(const AssetService&) = delete;
    AssetService& operator=(const AssetService&) = delete;

    void logAssetOperation(int assetId, const std::string& action,
                          const std::string& oldStatus, const std::string& newStatus,
                          const std::string& operatorName, const std::string& details = "");
};