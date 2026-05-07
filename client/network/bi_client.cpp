#include "bi_client.h"

BIClient::BIClient(std::shared_ptr<grpc::Channel> channel) {
    stub_ = bi::BIReportService::NewStub(channel);
}

bi::ReportResult BIClient::generateReport(int reportType, const bi::ReportFilter& filter) {
    bi::GenerateReportRequest req;
    req.set_report_type(reportType);
    *req.mutable_filter() = filter;
    
    bi::GenerateReportResponse resp;
    grpc::ClientContext ctx;
    
    grpc::Status status = stub_->GenerateReport(&ctx, req, &resp);
    
    if (status.ok() && resp.success()) {
        return resp.report();
    }
    
    return bi::ReportResult();
}

std::pair<std::string, std::string> BIClient::exportReport(const std::string& reportId, int format) {
    bi::ExportReportRequest req;
    req.set_report_id(reportId);
    req.set_export_format(format);
    
    bi::ExportReportResponse resp;
    grpc::ClientContext ctx;
    
    grpc::Status status = stub_->ExportReport(&ctx, req, &resp);
    
    if (status.ok() && resp.success()) {
        return {resp.data(), resp.filename()};
    }
    
    return {"", ""};
}

std::vector<bi::ReportResult> BIClient::getReports(int limit) {
    bi::GetReportsRequest req;
    req.set_limit(limit);
    
    bi::GetReportsResponse resp;
    grpc::ClientContext ctx;
    
    grpc::Status status = stub_->GetReports(&ctx, req, &resp);
    
    std::vector<bi::ReportResult> result;
    if (status.ok() && resp.success()) {
        for (const auto& report : resp.reports()) {
            result.push_back(report);
        }
    }
    
    return result;
}

bi::ReportResult BIClient::getReport(const std::string& reportId) {
    bi::GetReportRequest req;
    req.set_report_id(reportId);
    
    bi::ReportResult resp;
    grpc::ClientContext ctx;
    
    grpc::Status status = stub_->GetReport(&ctx, req, &resp);
    
    if (status.ok()) {
        return resp;
    }
    
    return bi::ReportResult();
}

bool BIClient::deleteReport(const std::string& reportId) {
    bi::DeleteReportRequest req;
    req.set_report_id(reportId);
    
    bi::DeleteReportResponse resp;
    grpc::ClientContext ctx;
    
    grpc::Status status = stub_->DeleteReport(&ctx, req, &resp);
    
    return status.ok() && resp.success();
}

std::string BIClient::addTask(const bi::ScheduledTask& task) {
    bi::AddTaskRequest req;
    *req.mutable_task() = task;
    
    bi::AddTaskResponse resp;
    grpc::ClientContext ctx;
    
    grpc::Status status = stub_->AddTask(&ctx, req, &resp);
    
    if (status.ok() && resp.success()) {
        return resp.task_id();
    }
    
    return "";
}

std::vector<bi::ScheduledTask> BIClient::getTasks() {
    bi::GetTasksRequest req;
    
    bi::GetTasksResponse resp;
    grpc::ClientContext ctx;
    
    grpc::Status status = stub_->GetTasks(&ctx, req, &resp);
    
    std::vector<bi::ScheduledTask> result;
    if (status.ok() && resp.success()) {
        for (const auto& task : resp.tasks()) {
            result.push_back(task);
        }
    }
    
    return result;
}

bool BIClient::updateTask(const std::string& taskId, const bi::ScheduledTask& task) {
    bi::UpdateTaskRequest req;
    req.set_task_id(taskId);
    *req.mutable_task() = task;
    
    bi::UpdateTaskResponse resp;
    grpc::ClientContext ctx;
    
    grpc::Status status = stub_->UpdateTask(&ctx, req, &resp);
    
    return status.ok() && resp.success();
}

bool BIClient::deleteTask(const std::string& taskId) {
    bi::DeleteTaskRequest req;
    req.set_task_id(taskId);
    
    bi::DeleteTaskResponse resp;
    grpc::ClientContext ctx;
    
    grpc::Status status = stub_->DeleteTask(&ctx, req, &resp);
    
    return status.ok() && resp.success();
}