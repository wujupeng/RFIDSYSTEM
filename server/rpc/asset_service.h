#pragma once
#include "asset.grpc.pb.h"
#include <grpcpp/grpcpp.h>

class AssetServiceImpl final : public asset::AssetService::Service {
public:
    grpc::Status CreateAsset(grpc::ServerContext* context,
                             const asset::CreateAssetRequest* request,
                             asset::CreateAssetResponse* response) override;

    grpc::Status GetAsset(grpc::ServerContext* context,
                          const asset::GetAssetRequest* request,
                          asset::GetAssetResponse* response) override;

    grpc::Status UpdateAssetStatus(grpc::ServerContext* context,
                                   const asset::UpdateAssetStatusRequest* request,
                                   asset::UpdateAssetStatusResponse* response) override;

    grpc::Status ListAssets(grpc::ServerContext* context,
                            const asset::ListAssetsRequest* request,
                            asset::ListAssetsResponse* response) override;

    grpc::Status BatchScanEPC(grpc::ServerContext* context,
                              const asset::BatchScanRequest* request,
                              asset::BatchScanResponse* response) override;

    grpc::Status StartInventoryTask(grpc::ServerContext* context,
                                    const asset::StartInventoryTaskRequest* request,
                                    asset::StartInventoryTaskResponse* response) override;

    grpc::Status GetInventoryTask(grpc::ServerContext* context,
                                  const asset::GetInventoryTaskRequest* request,
                                  asset::GetInventoryTaskResponse* response) override;

    grpc::Status HealthCheck(grpc::ServerContext* context,
                             const asset::HealthCheckRequest* request,
                             asset::HealthCheckResponse* response) override;

    grpc::Status GetMetrics(grpc::ServerContext* context,
                           const asset::GetMetricsRequest* request,
                           asset::GetMetricsResponse* response) override;
};