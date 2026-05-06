#pragma once
#include <memory>
#include <grpcpp/grpcpp.h>
#include "monitoring.grpc.pb.h"

class MonitoringClient {
public:
    MonitoringClient(std::shared_ptr<grpc::Channel> channel);

    monitoring::HealthResponse getHealth();
    monitoring::ActionDistributionResponse getDistribution();
    monitoring::AdoptionResponse getAdoption();

private:
    std::unique_ptr<monitoring::MonitoringService::Stub> stub_;
};
