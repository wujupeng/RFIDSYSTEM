#include "trajectory_service.h"
#include "../core/logger.h"

namespace rpc {

grpc::Status TrajectoryServiceImpl::GetTrajectory(
    grpc::ServerContext* context,
    const trajectory::TrajectoryRequest* request,
    trajectory::TrajectoryResponse* response) {
    
    spdlog::info("RPC: GetTrajectory called - asset_id={}", request->asset_id());
    
    try {
        auto trajectory = analytics::TrajectoryEngine::instance().getTrajectory(
            request->asset_id(),
            request->start_time(),
            request->end_time());
        
        response->set_asset_id(request->asset_id());
        
        for (const auto& point : trajectory.points) {
            auto* protoPoint = response->add_points();
            protoPoint->set_location(point.location);
            protoPoint->set_timestamp(point.timestamp);
            protoPoint->set_risk_score(0.0);
            protoPoint->set_status("normal");
        }
        
        return grpc::Status::OK;
        
    } catch (const std::exception& e) {
        spdlog::error("RPC: GetTrajectory failed - {}", e.what());
        return grpc::Status(grpc::StatusCode::INTERNAL, e.what());
    }
}

grpc::Status TrajectoryServiceImpl::StreamTrajectory(
    grpc::ServerContext* context,
    const trajectory::TrajectoryRequest* request,
    grpc::ServerWriter<trajectory::TrajectoryPoint>* writer) {
    
    spdlog::info("RPC: StreamTrajectory called - asset_id={}", request->asset_id());
    
    try {
        auto trajectory = analytics::TrajectoryEngine::instance().getTrajectory(
            request->asset_id(),
            request->start_time(),
            request->end_time());
        
        for (const auto& point : trajectory.points) {
            if (context->IsCancelled()) {
                spdlog::info("RPC: StreamTrajectory cancelled");
                break;
            }
            
            trajectory::TrajectoryPoint protoPoint;
            protoPoint.set_location(point.location);
            protoPoint.set_timestamp(point.timestamp);
            protoPoint.set_risk_score(0.0);
            protoPoint.set_status("normal");
            
            writer->Write(protoPoint);
            
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        
        spdlog::info("RPC: StreamTrajectory completed");
        return grpc::Status::OK;
        
    } catch (const std::exception& e) {
        spdlog::error("RPC: StreamTrajectory failed - {}", e.what());
        return grpc::Status(grpc::StatusCode::INTERNAL, e.what());
    }
}

} // namespace rpc