#include "asset_service.h"
#include "../db/db_pool.h"
#include "../core/logger.h"
#include <sstream>

AssetService& AssetService::instance() {
    static AssetService instance;
    return instance;
}

int AssetService::createAsset(const CreateAssetParams& params) {
    spdlog::info("Creating asset: name={}, type={}, asset_code={}, operator={}",
                 params.name, params.type, params.asset_code, params.operator_name);

    auto conn = DBPool::instance().acquire();
    pqxx::work W(*conn);

    std::string assetCode = params.asset_code.empty()
        ? AssetCodeGenerator::instance().generate()
        : params.asset_code;

    std::string sql = "INSERT INTO assets(name, type, asset_code, rfid_epc, location, status) "
                      "VALUES(" +
                      W.quote(params.name) + ", " +
                      W.quote(params.type) + ", " +
                      W.quote(assetCode) + ", " +
                      W.quote(params.rfid_epc) + ", " +
                      W.quote(params.location) + ", 'IN_STOCK') "
                      "RETURNING id";

    pqxx::result R = W.exec(sql);
    int id = R[0][0].as<int>();

    W.commit();

    logAssetOperation(id, "CREATE", "", "IN_STOCK", params.operator_name,
                      "Asset created with code: " + assetCode);

    spdlog::info("Asset created successfully: id={}, asset_code={}", id, assetCode);

    DBPool::instance().release(conn);
    return id;
}

Asset AssetService::getAsset(int id) {
    spdlog::debug("Fetching asset: id={}", id);

    auto conn = DBPool::instance().acquire();
    pqxx::work W(*conn);

    pqxx::result R = W.exec(
        "SELECT id, COALESCE(asset_code,''), name, type, COALESCE(rfid_epc,''), "
        "COALESCE(location,''), status, created_at FROM assets WHERE id = " +
        W.to_string(id)
    );

    Asset asset;
    if (R.empty()) {
        DBPool::instance().release(conn);
        throw AssetNotFoundException(id);
    }

    asset.id = R[0][0].as<int>();
    asset.asset_code = R[0][1].as<std::string>();
    asset.name = R[0][2].as<std::string>();
    asset.type = R[0][3].as<std::string>();
    asset.rfid_epc = R[0][4].as<std::string>();
    asset.location = R[0][5].as<std::string>();
    asset.status = AssetStatusMachine::stringToStatus(R[0][6].as<std::string>());

    DBPool::instance().release(conn);
    return asset;
}

bool AssetService::updateAssetStatus(int id, const std::string& newStatus, const std::string& operatorName) {
    spdlog::info("Updating asset status: id={}, new_status={}, operator={}",
                 id, newStatus, operatorName);

    if (!AssetStatusMachine::isValidStatus(newStatus)) {
        spdlog::warn("Invalid status: {}", newStatus);
        return false;
    }

    auto conn = DBPool::instance().acquire();
    pqxx::work W(*conn);

    pqxx::result R = W.exec(
        "SELECT status FROM assets WHERE id = " + W.to_string(id)
    );

    if (R.empty()) {
        DBPool::instance().release(conn);
        throw AssetNotFoundException(id);
    }

    std::string oldStatusStr = R[0][0].as<std::string>();
    AssetStatus oldStatus = AssetStatusMachine::stringToStatus(oldStatusStr);
    AssetStatus newStatusEnum = AssetStatusMachine::stringToStatus(newStatus);

    if (!AssetStatusMachine::canTransition(oldStatus, newStatusEnum)) {
        DBPool::instance().release(conn);
        throw InvalidStatusTransitionException(oldStatusStr, newStatus);
    }

    W.exec("UPDATE assets SET status = " + W.quote(newStatus) +
           ", updated_at = CURRENT_TIMESTAMP WHERE id = " + W.to_string(id));

    W.commit();

    logAssetOperation(id, "STATUS_CHANGE", oldStatusStr, newStatus, operatorName);

    spdlog::info("Asset status updated: id={}, {} -> {}", id, oldStatusStr, newStatus);

    DBPool::instance().release(conn);
    return true;
}

std::vector<Asset> AssetService::listAssets(const ListAssetsParams& params) {
    spdlog::debug("Listing assets: page={}, page_size={}, status_filter={}",
                  params.page, params.page_size, params.status_filter);

    auto conn = DBPool::instance().acquire();
    pqxx::work W(*conn);

    std::string whereClause;
    if (!params.status_filter.empty()) {
        whereClause = " WHERE status = " + W.quote(params.status_filter);
    }

    int offset = (params.page - 1) * params.page_size;
    std::string sql = "SELECT id, COALESCE(asset_code,''), name, type, COALESCE(rfid_epc,''), "
                      "COALESCE(location,''), status, created_at FROM assets" +
                      whereClause +
                      " ORDER BY created_at DESC LIMIT " + W.to_string(params.page_size) +
                      " OFFSET " + W.to_string(offset);

    pqxx::result R = W.exec(sql);

    std::vector<Asset> assets;
    for (const auto& row : R) {
        Asset asset;
        asset.id = row[0].as<int>();
        asset.asset_code = row[1].as<std::string>();
        asset.name = row[2].as<std::string>();
        asset.type = row[3].as<std::string>();
        asset.rfid_epc = row[4].as<std::string>();
        asset.location = row[5].as<std::string>();
        asset.status = AssetStatusMachine::stringToStatus(row[6].as<std::string>());
        assets.push_back(asset);
    }

    DBPool::instance().release(conn);
    return assets;
}

int AssetService::getTotalCount(const std::string& statusFilter) {
    auto conn = DBPool::instance().acquire();
    pqxx::work W(*conn);

    std::string whereClause;
    if (!statusFilter.empty()) {
        whereClause = " WHERE status = " + W.quote(statusFilter);
    }

    pqxx::result R = W.exec("SELECT COUNT(*) FROM assets" + whereClause);
    int count = R[0][0].as<int>();

    DBPool::instance().release(conn);
    return count;
}

void AssetService::logAssetOperation(int assetId, const std::string& action,
                                    const std::string& oldStatus, const std::string& newStatus,
                                    const std::string& operatorName, const std::string& details) {
    try {
        auto conn = DBPool::instance().acquire();
        pqxx::work W(*conn);

        std::string sql = "INSERT INTO asset_logs(asset_id, action, old_status, new_status, operator, details) "
                         "VALUES(" +
                         W.to_string(assetId) + ", " +
                         W.quote(action) + ", " +
                         W.quote(oldStatus) + ", " +
                         W.quote(newStatus) + ", " +
                         W.quote(operatorName) + ", " +
                         W.quote(details) + ")";

        W.exec(sql);
        W.commit();

        DBPool::instance().release(conn);
    } catch (const std::exception& e) {
        spdlog::error("Failed to log asset operation: {}", e.what());
    }
}