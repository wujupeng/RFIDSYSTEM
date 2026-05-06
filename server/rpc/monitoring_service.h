#pragma once
#include "monitoring.grpc.pb.h"
#include <grpcpp/grpcpp.h>

class MonitoringServiceImpl final : public monitoring::MonitoringService::Service {
public:
    grpc::Status GetSystemHealth(
        grpc::ServerContext* context,
        const monitoring::Empty* request,
        monitoring::HealthResponse* response) override;

    grpc::Status GetActionDistribution(
        grpc::ServerContext* context,
        const monitoring::Empty* request,
        monitoring::ActionDistributionResponse* response) override;

    grpc::Status GetAdoptionRate(
        grpc::ServerContext* context,
        const monitoring::Empty* request,
        monitoring::AdoptionResponse* response) override;
};
