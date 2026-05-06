#include "asset_service.h"
#include "../services/asset_service.h"
#include "../models/asset.h"
#include "../core/logger.h"

grpc::Status AssetServiceImpl::CreateAsset(
    grpc::ServerContext*,
    const asset::CreateAssetRequest* req,
    asset::CreateAssetResponse* res) {

    spdlog::info("RPC: CreateAsset called - name={}, type={}",
                 req->name(), req->type());

    try {
        AssetService::CreateAssetParams params;
        params.name = req->name();
        params.type = req->type();
        params.asset_code = req->asset_code();
        params.rfid_epc = req->rfid_epc();
        params.location = req->location();
        params.operator_name = req->operator_name();

        int id = AssetService::instance().createAsset(params);

        res->set_id(id);
        res->set_message("Asset created successfully");

        spdlog::info("RPC: CreateAsset succeeded - id={}", id);
        return grpc::Status::OK;

    } catch (const std::exception& e) {
        spdlog::error("RPC: CreateAsset failed - {}", e.what());
        return grpc::Status(grpc::StatusCode::INTERNAL, e.what());
    }
}

grpc::Status AssetServiceImpl::GetAsset(
    grpc::ServerContext*,
    const asset::GetAssetRequest* req,
    asset::GetAssetResponse* res) {

    spdlog::info("RPC: GetAsset called - id={}", req->id());

    try {
        Asset asset = AssetService::instance().getAsset(req->id());

        res->set_id(asset.id);
        res->set_name(asset.name);
        res->set_type(asset.type);
        res->set_asset_code(asset.asset_code);
        res->set_rfid_epc(asset.rfid_epc);
        res->set_location(asset.location);
        res->set_status(AssetStatusMachine::statusToString(asset.status));

        spdlog::info("RPC: GetAsset succeeded - id={}", asset.id);
        return grpc::Status::OK;

    } catch (const std::exception& e) {
        spdlog::error("RPC: GetAsset failed - {}", e.what());
        return grpc::Status(grpc::StatusCode::INTERNAL, e.what());
    }
}

grpc::Status AssetServiceImpl::UpdateAssetStatus(
    grpc::ServerContext*,
    const asset::UpdateAssetStatusRequest* req,
    asset::UpdateAssetStatusResponse* res) {

    spdlog::info("RPC: UpdateAssetStatus called - id={}, new_status={}, operator={}",
                 req->id(), req->new_status(), req->operator_name());

    try {
        bool success = AssetService::instance().updateAssetStatus(
            req->id(),
            req->new_status(),
            req->operator_name()
        );

        res->set_success(success);
        res->set_message(success ? "Status updated successfully" : "Invalid status transition");

        spdlog::info("RPC: UpdateAssetStatus completed - id={}, success={}", req->id(), success);
        return grpc::Status::OK;

    } catch (const std::exception& e) {
        spdlog::error("RPC: UpdateAssetStatus failed - {}", e.what());
        return grpc::Status(grpc::StatusCode::INTERNAL, e.what());
    }
}

grpc::Status AssetServiceImpl::ListAssets(
    grpc::ServerContext*,
    const asset::ListAssetsRequest* req,
    asset::ListAssetsResponse* res) {

    spdlog::info("RPC: ListAssets called - page={}, page_size={}",
                 req->page(), req->page_size());

    try {
        AssetService::ListAssetsParams params;
        params.page = req->page();
        params.page_size = req->page_size();
        params.status_filter = req->status_filter();

        std::vector<Asset> assets = AssetService::instance().listAssets(params);
        int total = AssetService::instance().getTotalCount(params.status_filter);

        for (const auto& asset : assets) {
            auto* assetInfo = res->add_assets();
            assetInfo->set_id(asset.id);
            assetInfo->set_name(asset.name);
            assetInfo->set_type(asset.type);
            assetInfo->set_asset_code(asset.asset_code);
            assetInfo->set_rfid_epc(asset.rfid_epc);
            assetInfo->set_location(asset.location);
            assetInfo->set_status(AssetStatusMachine::statusToString(asset.status));
        }

        res->set_total_count(total);

        spdlog::info("RPC: ListAssets succeeded - returned {} assets, total={}",
                     assets.size(), total);
        return grpc::Status::OK;

    } catch (const std::exception& e) {
        spdlog::error("RPC: ListAssets failed - {}", e.what());
        return grpc::Status(grpc::StatusCode::INTERNAL, e.what());
    }
}

grpc::Status AssetServiceImpl::BatchScanEPC(
    grpc::ServerContext*,
    const asset::BatchScanRequest* req,
    asset::BatchScanResponse* res) {
    res->set_total_scanned(0);
    return grpc::Status::OK;
}

grpc::Status AssetServiceImpl::StartInventoryTask(
    grpc::ServerContext*,
    const asset::StartInventoryTaskRequest* req,
    asset::StartInventoryTaskResponse* res) {
    res->set_task_id(0);
    return grpc::Status::OK;
}

grpc::Status AssetServiceImpl::GetInventoryTask(
    grpc::ServerContext*,
    const asset::GetInventoryTaskRequest* req,
    asset::GetInventoryTaskResponse* res) {
    return grpc::Status::OK;
}

grpc::Status AssetServiceImpl::HealthCheck(
    grpc::ServerContext*,
    const asset::HealthCheckRequest* req,
    asset::HealthCheckResponse* res) {
    res->set_status("OK");
    res->set_version("2.0");
    res->set_uptime_seconds(0);
    res->set_active_readers(0);
    res->set_db_pool_available(10);
    res->set_db_pool_size(10);
    return grpc::Status::OK;
}

grpc::Status AssetServiceImpl::GetAssetStatistics(
    grpc::ServerContext*,
    const asset::GetAssetStatisticsRequest* req,
    asset::GetAssetStatisticsResponse* res) {

    spdlog::info("RPC: GetAssetStatistics called");

    try {
        AssetStatistics stats = AssetService::instance().getAssetStatistics();

        res->set_total_assets(stats.total_assets);
        res->set_in_stock_count(stats.in_stock_count);
        res->set_in_use_count(stats.in_use_count);
        res->set_repair_count(stats.repair_count);
        res->set_scrapped_count(stats.scrapped_count);

        spdlog::info("RPC: GetAssetStatistics succeeded");
        return grpc::Status::OK;

    } catch (const std::exception& e) {
        spdlog::error("RPC: GetAssetStatistics failed - {}", e.what());
        return grpc::Status(grpc::StatusCode::INTERNAL, e.what());
    }
}

grpc::Status AssetServiceImpl::GetMetrics(
    grpc::ServerContext*,
    const asset::GetMetricsRequest* req,
    asset::GetMetricsResponse* res) {
    res->set_epc_per_second(0);
    res->set_db_latency_p99_ms(0);
    res->set_request_success_rate(100);
    res->set_error_rate(0);
    res->set_active_readers(0);
    res->set_active_tasks(0);
    return grpc::Status::OK;
}
