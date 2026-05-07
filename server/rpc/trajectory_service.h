#pragma once
#include "trajectory.grpc.pb.h"
#include "../analytics/trajectory_engine.h"

namespace rpc {

class TrajectoryServiceImpl final : public trajectory::TrajectoryService::Service {
public:
    grpc::Status GetTrajectory(
        grpc::ServerContext* context,
        const trajectory::TrajectoryRequest* request,
        trajectory::TrajectoryResponse* response) override;
    
    grpc::Status StreamTrajectory(
        grpc::ServerContext* context,
        const trajectory::TrajectoryRequest* request,
        grpc::ServerWriter<trajectory::TrajectoryPoint>* writer) override;
};

} // namespace rpc