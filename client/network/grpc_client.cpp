#include "grpc_client.h"
#include "asset.grpc.pb.h"
#include "decision.grpc.pb.h"
#include <grpcpp/grpcpp.h>

class GrpcClient::Impl {
public:
    std::unique_ptr<asset::AssetService::Stub> asset_stub_;
    std::unique_ptr<decision::DecisionService::Stub> decision_stub_;
};

GrpcClient::GrpcClient(const std::string& server_address) : pImpl_(new Impl) {
    auto channel = grpc::CreateChannel(server_address, grpc::InsecureChannelCredentials());
    pImpl_->asset_stub_ = asset::AssetService::NewStub(channel);
    pImpl_->decision_stub_ = decision::DecisionService::NewStub(channel);
}

GrpcClient::~GrpcClient() = default;

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

    auto status = pImpl_->asset_stub_->CreateAsset(&ctx, req, &res);
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
    if (pImpl_->asset_stub_->GetAsset(&ctx, req, &res).ok()) {
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

    auto status = pImpl_->asset_stub_->UpdateAssetStatus(&ctx, req, &res);
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
    if (pImpl_->asset_stub_->ListAssets(&ctx, req, &res).ok()) {
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

    auto status = pImpl_->asset_stub_->StartInventoryTask(&ctx, req, &res);
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
    if (pImpl_->asset_stub_->BatchScanEPC(&ctx, req, &res).ok()) {
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
    if (pImpl_->asset_stub_->GetInventoryTask(&ctx, req, &res).ok()) {
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

DecisionsResponse GrpcClient::getRecentDecisions(int limit, const std::string& userFilter) {
    decision::GetDecisionsRequest req;
    req.set_limit(limit);
    req.set_user_filter(userFilter);

    decision::GetDecisionsResponse res;
    grpc::ClientContext ctx;

    DecisionsResponse result{};
    if (pImpl_->decision_stub_->GetDecisions(&ctx, req, &res).ok()) {
        for (const auto& d : res.decisions()) {
            DecisionView dv;
            dv.asset_id = d.asset_id();
            dv.asset_name = d.asset_name();
            dv.location = d.location();
            dv.action = d.action();
            dv.risk_level = d.risk_level();
            dv.reason = d.reason();
            dv.timestamp = d.timestamp();
            dv.is_handled = d.is_handled();
            dv.is_executed = d.is_executed();
            dv.is_ignored = d.is_ignored();
            result.decisions.push_back(dv);
        }
        result.total_count = res.total_count();
        result.adoption_rate = res.adoption_rate();
    }
    return result;
}

bool GrpcClient::reportDecision(int assetId, bool executed, bool ignored, const std::string& userName) {
    decision::ReportDecisionRequest req;
    req.set_asset_id(assetId);
    req.set_executed(executed);
    req.set_ignored(ignored);
    req.set_operator_name(userName);

    decision::ReportDecisionResponse res;
    grpc::ClientContext ctx;

    auto status = pImpl_->decision_stub_->ReportDecision(&ctx, req, &res);
    return status.ok() && res.success();
}
