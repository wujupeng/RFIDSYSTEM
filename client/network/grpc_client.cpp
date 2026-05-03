#include "grpc_client.h"
#include "asset.grpc.pb.h"
#include <grpcpp/grpcpp.h>

class GrpcClient::Impl {
public:
    std::unique_ptr<asset::AssetService::Stub> stub_;
};

GrpcClient::GrpcClient(const std::string& server_address) : pImpl_(new Impl) {
    auto channel = grpc::CreateChannel(server_address, grpc::InsecureChannelCredentials());
    pImpl_->stub_ = asset::AssetService::NewStub(channel);
}

int GrpcClient::createAsset(const CreateAssetParams& params) {
    asset::CreateAssetRequest req;
    req.set_name(params.name);
    req.set_type(params.type);
    req.set_asset_code(params.asset_code);
    req.set_rfid_epc(params.rfid_epc);
    req.set_location(params.location);
    req.set_operator_name(params.operator_name);

    asset::CreateAssetResponse res;
    grpc::ClientContext ctx;

    auto status = pImpl_->stub_->CreateAsset(&ctx, req, &res);
    if (status.ok()) {
        return res.id();
    }
    return -1;
}

Asset GrpcClient::getAsset(int id) {
    asset::GetAssetRequest req;
    req.set_id(id);

    asset::GetAssetResponse res;
    grpc::ClientContext ctx;

    Asset asset{};
    if (pImpl_->stub_->GetAsset(&ctx, req, &res).ok()) {
        asset.id = res.id();
        asset.name = res.name();
        asset.type = res.type();
        asset.asset_code = res.asset_code();
        asset.rfid_epc = res.rfid_epc();
        asset.location = res.location();
        asset.status = res.status();
    }
    return asset;
}

bool GrpcClient::updateAssetStatus(int id, const std::string& newStatus, const std::string& operatorName) {
    asset::UpdateAssetStatusRequest req;
    req.set_id(id);
    req.set_new_status(newStatus);
    req.set_operator_name(operatorName);

    asset::UpdateAssetStatusResponse res;
    grpc::ClientContext ctx;

    auto status = pImpl_->stub_->UpdateAssetStatus(&ctx, req, &res);
    return status.ok() && res.success();
}

std::vector<Asset> GrpcClient::listAssets(const ListAssetsParams& params) {
    asset::ListAssetsRequest req;
    req.set_page(params.page);
    req.set_page_size(params.page_size);
    req.set_status_filter(params.status_filter);

    asset::ListAssetsResponse res;
    grpc::ClientContext ctx;

    std::vector<Asset> assets;
    if (pImpl_->stub_->ListAssets(&ctx, req, &res).ok()) {
        for (const auto& a : res.assets()) {
            Asset asset{};
            asset.id = a.id();
            asset.name = a.name();
            asset.type = a.type();
            asset.asset_code = a.asset_code();
            asset.rfid_epc = a.rfid_epc();
            asset.location = a.location();
            asset.status = a.status();
            assets.push_back(asset);
        }
    }
    return assets;
}

int GrpcClient::startInventoryTask(const std::string& taskName, const std::string& location, const std::string& operatorName) {
    asset::StartInventoryTaskRequest req;
    req.set_task_name(taskName);
    req.set_location(location);
    req.set_operator_name(operatorName);

    asset::StartInventoryTaskResponse res;
    grpc::ClientContext ctx;

    auto status = pImpl_->stub_->StartInventoryTask(&ctx, req, &res);
    if (status.ok()) {
        return res.task_id();
    }
    return -1;
}

BatchScanResult GrpcClient::batchScanEPC(const std::vector<std::string>& epcs, int taskId) {
    asset::BatchScanRequest req;
    for (const auto& epc : epcs) {
        req.add_epcs(epc);
    }
    req.set_task_id(taskId);

    asset::BatchScanResponse res;
    grpc::ClientContext ctx;

    BatchScanResult result{};
    if (pImpl_->stub_->BatchScanEPC(&ctx, req, &res).ok()) {
        for (const auto& f : res.found()) {
            ScanResult sr;
            sr.epc = f.epc();
            sr.asset_id = f.asset_id();
            sr.asset_code = f.asset_code();
            sr.asset_name = f.asset_name();
            sr.status = f.status();
            result.found.push_back(sr);
        }
        for (const auto& m : res.missing()) {
            result.missing.push_back(m);
        }
        for (const auto& e : res.extra()) {
            result.extra.push_back(e);
        }
        result.total_scanned = res.total_scanned();
        result.total_found = res.total_found();
    }
    return result;
}

InventoryTask GrpcClient::getInventoryTask(int taskId) {
    asset::GetInventoryTaskRequest req;
    req.set_task_id(taskId);

    asset::GetInventoryTaskResponse res;
    grpc::ClientContext ctx;

    InventoryTask task{};
    if (pImpl_->stub_->GetInventoryTask(&ctx, req, &res).ok()) {
        const auto& t = res.task();
        task.id = t.id();
        task.task_name = t.task_name();
        task.status = t.status();
        task.scanned_count = t.scanned_count();
        task.found_count = t.found_count();
        task.missing_count = t.missing_count();
        task.extra_count = t.extra_count();
        task.location = t.location();
    }
    return task;
}

bool GrpcClient::completeInventoryTask(int taskId) {
    return true;
}