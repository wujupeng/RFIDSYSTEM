#pragma once
#include "decision.grpc.pb.h"
#include <grpcpp/grpcpp.h>
#include <memory>

class DecisionServiceImpl final : public decision::DecisionService::Service {
public:
    grpc::Status GetDecisions(grpc::ServerContext* context,
                              const decision::GetDecisionsRequest* request,
                              decision::GetDecisionsResponse* response) override;

    grpc::Status ReportDecision(grpc::ServerContext* context,
                               const decision::ReportDecisionRequest* request,
                               decision::ReportDecisionResponse* response) override;
};
