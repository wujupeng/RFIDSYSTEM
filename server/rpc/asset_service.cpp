#include "asset_service.h"
#include "../services/asset_service.h"
#include "../services/inventory_service.h"
#include "../models/asset.h"
#include "../core/logger.h"
#include "../core/rate_limiter.h"
#include "../core/metrics.h"
#include "../rfid/reader.h"
#include "../db/db_pool.h"

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

    } catch (const AssetNotFoundException& e) {
        spdlog::error("RPC: CreateAsset - {}", e.what());
        return grpc::Status(grpc::StatusCode::NOT_FOUND, e.what());

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

    } catch (const AssetNotFoundException& e) {
        spdlog::warn("RPC: GetAsset - {}", e.what());
        return grpc::Status(grpc::StatusCode::NOT_FOUND, e.what());

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

    } catch (const AssetNotFoundException& e) {
        spdlog::warn("RPC: UpdateAssetStatus - {}", e.what());
        return grpc::Status(grpc::StatusCode::NOT_FOUND, e.what());

    } catch (const InvalidStatusTransitionException& e) {
        spdlog::warn("RPC: UpdateAssetStatus - {}", e.what());
        return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, e.what());

    } catch (const std::exception& e) {
        spdlog::error("RPC: UpdateAssetStatus failed - {}", e.what());
        return grpc::Status(grpc::StatusCode::INTERNAL, e.what());
    }
}

grpc::Status AssetServiceImpl::ListAssets(
    grpc::ServerContext*,
    const asset::ListAssetsRequest* req,
    asset::ListAssetsResponse* res) {

    spdlog::info("RPC: ListAssets called - page={}, page_size={}, status_filter={}",
                 req->page(), req->page_size(), req->status_filter());

    try {
        AssetService::ListAssetsParams params;
        params.page = req->page() > 0 ? req->page() : 1;
        params.page_size = req->page_size() > 0 ? req->page_size() : 20;
        params.status_filter = req->status_filter();

        std::vector<Asset> assets = AssetService::instance().listAssets(params);
        int totalCount = AssetService::instance().getTotalCount(params.status_filter);

        for (const auto& asset : assets) {
            auto* assetProto = res->add_assets();
            assetProto->set_id(asset.id);
            assetProto->set_name(asset.name);
            assetProto->set_type(asset.type);
            assetProto->set_asset_code(asset.asset_code);
            assetProto->set_rfid_epc(asset.rfid_epc);
            assetProto->set_location(asset.location);
            assetProto->set_status(AssetStatusMachine::statusToString(asset.status));
        }

        res->set_total_count(totalCount);

        spdlog::info("RPC: ListAssets succeeded - returned {} of {} total",
                     assets.size(), totalCount);
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

    spdlog::info("RPC: BatchScanEPC called - epc_count={}, task_id={}",
                 req->epcs_size(), req->task_id());

    try {
        std::vector<std::string> epcs;
        for (const auto& epc : req->epcs()) {
            epcs.push_back(epc);
        }

        auto details = InventoryService::instance().scanEPCsWithDetails(epcs, req->task_id());

        for (const auto& detail : details) {
            auto* scanResult = res->add_found();
            scanResult->set_epc(detail.epc);
            scanResult->set_asset_id(detail.asset_id);
            scanResult->set_asset_code(detail.asset_code);
            scanResult->set_asset_name(detail.asset_name);
            scanResult->set_status(detail.status);
        }

        res->set_total_scanned(epcs.size());
        res->set_total_found(details.size());

        spdlog::info("RPC: BatchScanEPC succeeded - scanned={}, found={}",
                     epcs.size(), details.size());
        return grpc::Status::OK;

    } catch (const std::exception& e) {
        spdlog::error("RPC: BatchScanEPC failed - {}", e.what());
        return grpc::Status(grpc::StatusCode::INTERNAL, e.what());
    }
}

grpc::Status AssetServiceImpl::StartInventoryTask(
    grpc::ServerContext*,
    const asset::StartInventoryTaskRequest* req,
    asset::StartInventoryTaskResponse* res) {

    spdlog::info("RPC: StartInventoryTask called - name={}, location={}, operator={}",
                 req->task_name(), req->location(), req->operator_name());

    try {
        int taskId = InventoryService::instance().startTask(
            req->task_name(),
            req->location(),
            req->operator_name()
        );

        res->set_task_id(taskId);
        res->set_message("Inventory task started");

        spdlog::info("RPC: StartInventoryTask succeeded - task_id={}", taskId);
        return grpc::Status::OK;

    } catch (const std::exception& e) {
        spdlog::error("RPC: StartInventoryTask failed - {}", e.what());
        return grpc::Status(grpc::StatusCode::INTERNAL, e.what());
    }
}

grpc::Status AssetServiceImpl::GetInventoryTask(
    grpc::ServerContext*,
    const asset::GetInventoryTaskRequest* req,
    asset::GetInventoryTaskResponse* res) {

    spdlog::info("RPC: GetInventoryTask called - task_id={}", req->task_id());

    try {
        InventoryTask task = InventoryService::instance().getTask(req->task_id());

        auto* taskProto = res->mutable_task();
        taskProto->set_id(task.id);
        taskProto->set_task_name(task.task_name);
        taskProto->set_status(task.status);
        taskProto->set_scanned_count(task.scanned_count);
        taskProto->set_found_count(task.found_count);
        taskProto->set_missing_count(task.missing_count);
        taskProto->set_extra_count(task.extra_count);
        taskProto->set_location(task.location);
        taskProto->set_operator_name(task.operator_name);

        spdlog::info("RPC: GetInventoryTask succeeded - task_id={}", task.id);
        return grpc::Status::OK;

    } catch (const std::exception& e) {
        spdlog::error("RPC: GetInventoryTask failed - {}", e.what());
        return grpc::Status(grpc::StatusCode::INTERNAL, e.what());
    }
}

grpc::Status AssetServiceImpl::HealthCheck(
    grpc::ServerContext*,
    const asset::HealthCheckRequest*,
    asset::HealthCheckResponse* res) {

    spdlog::debug("RPC: HealthCheck called");

    res->set_status("OK");
    res->set_version("1.2.0");
    res->set_uptime_seconds(ServerStats::instance().getUptimeSeconds());
    res->set_active_readers(rfid::ReaderManager::instance().getConnectedReaderCount());
    res->set_db_pool_available(DBPool::instance().availableCount());
    res->set_db_pool_size(DBPool::instance().poolSize());

    return grpc::Status::OK;
}

grpc::Status AssetServiceImpl::GetMetrics(
    grpc::ServerContext*,
    const asset::GetMetricsRequest*,
    asset::GetMetricsResponse* res) {

    auto& metrics = Metrics::instance();

    res->set_uptime_seconds(ServerStats::instance().getUptimeSeconds());
    res->set_epc_per_second(metrics.getEPCPerSecond());
    res->set_db_latency_p99_ms(metrics.getDBLatencyP99());
    res->set_request_success_rate(metrics.getRequestSuccessRate());
    res->set_error_rate(metrics.getErrorRate());
    res->set_active_readers(metrics.getActiveReaders());
    res->set_active_tasks(metrics.getActiveTasks());
    res->set_db_pool_available(DBPool::instance().availableCount());
    res->set_db_pool_size(DBPool::instance().poolSize());

    return grpc::Status::OK;
}