#pragma once
#include <memory>
#include <vector>
#include <string>
#include <grpcpp/grpcpp.h>
#include "trajectory.grpc.pb.h"

class TrajectoryClient {
public:
    TrajectoryClient(std::shared_ptr<grpc::Channel> channel);
    
    trajectory::TrajectoryResponse getTrajectory(int assetId, 
                                                  int64_t startTime = 0, 
                                                  int64_t endTime = 0);
    
    void streamTrajectory(int assetId, 
                          std::function<void(const trajectory::TrajectoryPoint&)> callback);

private:
    std::unique_ptr<trajectory::TrajectoryService::Stub> stub_;
};