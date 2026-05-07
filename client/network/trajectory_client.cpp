#include "trajectory_client.h"

TrajectoryClient::TrajectoryClient(std::shared_ptr<grpc::Channel> channel) {
    stub_ = trajectory::TrajectoryService::NewStub(channel);
}

trajectory::TrajectoryResponse TrajectoryClient::getTrajectory(int assetId, 
                                                                int64_t startTime, 
                                                                int64_t endTime) {
    trajectory::TrajectoryRequest request;
    request.set_asset_id(assetId);
    request.set_start_time(startTime);
    request.set_end_time(endTime);
    
    trajectory::TrajectoryResponse response;
    grpc::ClientContext context;
    
    grpc::Status status = stub_->GetTrajectory(&context, request, &response);
    
    if (!status.ok()) {
        throw std::runtime_error("Failed to get trajectory: " + status.error_message());
    }
    
    return response;
}

void TrajectoryClient::streamTrajectory(int assetId, 
                                        std::function<void(const trajectory::TrajectoryPoint&)> callback) {
    trajectory::TrajectoryRequest request;
    request.set_asset_id(assetId);
    
    grpc::ClientContext context;
    
    std::unique_ptr<grpc::ClientReader<trajectory::TrajectoryPoint>> reader(
        stub_->StreamTrajectory(&context, request));
    
    trajectory::TrajectoryPoint point;
    while (reader->Read(&point)) {
        callback(point);
    }
    
    grpc::Status status = reader->Finish();
    if (!status.ok()) {
        throw std::runtime_error("Stream failed: " + status.error_message());
    }
}