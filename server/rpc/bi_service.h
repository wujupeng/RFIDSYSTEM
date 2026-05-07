#pragma once
#include "bi.grpc.pb.h"
#include "../bi/bi_service.h"
#include "../bi/task_scheduler.h"

namespace rpc {

class BIReportServiceImpl final : public bi::BIReportService::Service {
public:
    grpc::Status GenerateReport(
        grpc::ServerContext* context,
        const bi::GenerateReportRequest* request,
        bi::GenerateReportResponse* response) override;

    grpc::Status ExportReport(
        grpc::ServerContext* context,
        const bi::ExportReportRequest* request,
        bi::ExportReportResponse* response) override;

    grpc::Status GetReports(
        grpc::ServerContext* context,
        const bi::GetReportsRequest* request,
        bi::GetReportsResponse* response) override;

    grpc::Status GetReport(
        grpc::ServerContext* context,
        const bi::GetReportRequest* request,
        bi::ReportResult* response) override;

    grpc::Status DeleteReport(
        grpc::ServerContext* context,
        const bi::DeleteReportRequest* request,
        bi::DeleteReportResponse* response) override;

    grpc::Status AddTask(
        grpc::ServerContext* context,
        const bi::AddTaskRequest* request,
        bi::AddTaskResponse* response) override;

    grpc::Status GetTasks(
        grpc::ServerContext* context,
        const bi::GetTasksRequest* request,
        bi::GetTasksResponse* response) override;

    grpc::Status UpdateTask(
        grpc::ServerContext* context,
        const bi::UpdateTaskRequest* request,
        bi::UpdateTaskResponse* response) override;

    grpc::Status DeleteTask(
        grpc::ServerContext* context,
        const bi::DeleteTaskRequest* request,
        bi::DeleteTaskResponse* response) override;

private:
    bi::ReportResult convertToProto(const bi::ReportResult& result);
    bi::ReportSection convertSectionToProto(const bi::ReportSection& section);
    bi::ReportDataPoint convertDataPointToProto(const bi::ReportDataPoint& dp);
    bi::ReportFilter convertFilterFromProto(const bi::ReportFilter& filter);
};

} // namespace rpc