#include "decision_service.h"
#include "repository/decision_repository.h"
#include "analytics/decision/decision_stability.h"
#include <spdlog/spdlog.h>

grpc::Status DecisionServiceImpl::GetDecisions(
    grpc::ServerContext* context,
    const decision::GetDecisionsRequest* request,
    decision::GetDecisionsResponse* response)
{
    int limit = request->limit();
    if (limit <= 0 || limit > 1000) {
        limit = 100;
    }

    auto& repo = DecisionRepository::instance();
    auto decisions = repo.getRecent(limit);

    double adoption_rate = repo.getAdoptionRate(24);

    response->set_total_count(decisions.size());
    response->set_adoption_rate(adoption_rate);

    for (const auto& rec : decisions) {
        auto* item = response->add_decisions();
        item->set_asset_id(rec.asset_id);
        item->set_asset_name(rec.asset_name);
        item->set_location(rec.location);
        item->set_action(rec.action);
        item->set_risk_level(rec.risk_level);
        item->set_reason(rec.reason);
        item->set_timestamp(rec.created_at);
        item->set_is_handled(rec.executed || rec.ignored);
        item->set_is_executed(rec.executed);
        item->set_is_ignored(rec.ignored);
    }

    return grpc::Status::OK;
}

grpc::Status DecisionServiceImpl::ReportDecision(
    grpc::ServerContext* context,
    const decision::ReportDecisionRequest* request,
    decision::ReportDecisionResponse* response)
{
    int assetId = request->asset_id();
    bool executed = request->executed();
    bool ignored = request->ignored();
    std::string operatorName = request->operator_name();

    spdlog::info("ReportDecision: asset_id={}, executed={}, ignored={}, operator={}",
                  assetId, executed, ignored, operatorName);

    auto& repo = DecisionRepository::instance();
    repo.markExecuted(assetId, executed, ignored, operatorName);

    response->set_success(true);
    response->set_message("Decision recorded successfully");

    return grpc::Status::OK;
}
