#pragma once
#include <memory>
#include <vector>
#include <string>
#include <grpcpp/grpcpp.h>
#include "bi.grpc.pb.h"

class BIClient {
public:
    BIClient(std::shared_ptr<grpc::Channel> channel);
    
    bi::ReportResult generateReport(int reportType, const bi::ReportFilter& filter);
    std::pair<std::string, std::string> exportReport(const std::string& reportId, int format);
    std::vector<bi::ReportResult> getReports(int limit = 10);
    bi::ReportResult getReport(const std::string& reportId);
    bool deleteReport(const std::string& reportId);
    
    std::string addTask(const bi::ScheduledTask& task);
    std::vector<bi::ScheduledTask> getTasks();
    bool updateTask(const std::string& taskId, const bi::ScheduledTask& task);
    bool deleteTask(const std::string& taskId);

private:
    std::unique_ptr<bi::BIReportService::Stub> stub_;
};