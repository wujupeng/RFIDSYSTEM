#include "monitoring_client.h"

MonitoringClient::MonitoringClient(std::shared_ptr<grpc::Channel> channel) {
    stub_ = monitoring::MonitoringService::NewStub(channel);
}

monitoring::HealthResponse MonitoringClient::getHealth() {
    monitoring::Empty req;
    monitoring::HealthResponse resp;
    grpc::ClientContext ctx;
    auto deadline = std::chrono::system_clock::now() + std::chrono::milliseconds(1000);
    ctx.set_deadline(deadline);
    auto status = stub_->GetSystemHealth(&ctx, req, &resp);
    if (!status.ok()) {
        resp.set_server_ok(false);
        resp.set_grpc_ok(false);
        resp.set_bandit_ok(false);
    }
    return resp;
}

monitoring::ActionDistributionResponse MonitoringClient::getDistribution() {
    monitoring::Empty req;
    monitoring::ActionDistributionResponse resp;
    grpc::ClientContext ctx;
    auto deadline = std::chrono::system_clock::now() + std::chrono::milliseconds(1000);
    ctx.set_deadline(deadline);
    stub_->GetActionDistribution(&ctx, req, &resp);
    return resp;
}

monitoring::AdoptionResponse MonitoringClient::getAdoption() {
    monitoring::Empty req;
    monitoring::AdoptionResponse resp;
    grpc::ClientContext ctx;
    auto deadline = std::chrono::system_clock::now() + std::chrono::milliseconds(1000);
    ctx.set_deadline(deadline);
    stub_->GetAdoptionRate(&ctx, req, &resp);
    return resp;
}
