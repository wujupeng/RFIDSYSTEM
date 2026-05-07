#include "bi_service.h"
#include "../core/logger.h"

namespace rpc {

grpc::Status BIReportServiceImpl::GenerateReport(
    grpc::ServerContext* context,
    const bi::GenerateReportRequest* request,
    bi::GenerateReportResponse* response) {
    
    spdlog::info("RPC: GenerateReport called");
    
    try {
        bi::InternalReportFilter filter;
        filter.start_date = request->filter().start_date();
        filter.end_date = request->filter().end_date();
        filter.limit = request->filter().limit();
        
        for (const auto& cat : request->filter().asset_categories()) {
            filter.asset_categories.push_back(cat);
        }
        for (const auto& loc : request->filter().locations()) {
            filter.locations.push_back(loc);
        }
        for (const auto& status : request->filter().statuses()) {
            filter.statuses.push_back(status);
        }
        
        auto report = bi::ReportService::instance().generateReport(
            static_cast<bi::ReportType>(request->report_type()), filter);
        
        response->mutable_report()->set_report_id(report.report_id);
        response->mutable_report()->set_title(report.title);
        response->mutable_report()->set_generated_at(report.generated_at);
        response->mutable_report()->set_report_type(static_cast<int>(report.type));
        response->mutable_report()->set_total_assets(report.total_assets);
        response->mutable_report()->set_active_assets(report.active_assets);
        response->mutable_report()->set_inactive_assets(report.inactive_assets);
        
        for (const auto& section : report.sections) {
            auto protoSection = response->mutable_report()->add_sections();
            protoSection->set_title(section.title);
            protoSection->set_chart_type(section.chart_type);
            
            for (const auto& dp : section.data_points) {
                auto protoDp = protoSection->add_data_points();
                protoDp->set_label(dp.label);
                protoDp->set_value(dp.value);
                protoDp->set_unit(dp.unit);
            }
            
            for (const auto& row : section.table_data) {
                std::string rowStr;
                for (size_t i = 0; i < row.size(); ++i) {
                    if (i > 0) rowStr += ",";
                    rowStr += row[i];
                }
                protoSection->add_table_data(rowStr);
            }
        }
        
        response->set_success(true);
        response->set_message("Report generated successfully");
        
        return grpc::Status::OK;
        
    } catch (const std::exception& e) {
        spdlog::error("RPC: GenerateReport failed - {}", e.what());
        response->set_success(false);
        response->set_message(e.what());
        return grpc::Status(grpc::StatusCode::INTERNAL, e.what());
    }
}

grpc::Status BIReportServiceImpl::ExportReport(
    grpc::ServerContext* context,
    const bi::ExportReportRequest* request,
    bi::ExportReportResponse* response) {
    
    spdlog::info("RPC: ExportReport called - report_id={}", request->report_id());
    
    try {
        auto report = bi::ReportService::instance().getReportById(request->report_id());
        
        if (report.report_id.empty()) {
            response->set_success(false);
            response->set_message("Report not found");
            return grpc::Status(grpc::StatusCode::NOT_FOUND, "Report not found");
        }
        
        std::string data = bi::ReportService::instance().exportReport(
            report, static_cast<bi::ExportFormat>(request->export_format()));
        
        response->set_data(data);
        response->set_filename("report_" + request->report_id() + ".csv");
        response->set_success(true);
        response->set_message("Report exported successfully");
        
        return grpc::Status::OK;
        
    } catch (const std::exception& e) {
        spdlog::error("RPC: ExportReport failed - {}", e.what());
        response->set_success(false);
        response->set_message(e.what());
        return grpc::Status(grpc::StatusCode::INTERNAL, e.what());
    }
}

grpc::Status BIReportServiceImpl::GetReports(
    grpc::ServerContext* context,
    const bi::GetReportsRequest* request,
    bi::GetReportsResponse* response) {
    
    spdlog::info("RPC: GetReports called");
    
    try {
        int limit = request->limit() > 0 ? request->limit() : 10;
        auto reports = bi::ReportService::instance().getRecentReports(limit);
        
        for (const auto& report : reports) {
            auto protoReport = response->add_reports();
            protoReport->set_report_id(report.report_id);
            protoReport->set_title(report.title);
            protoReport->set_generated_at(report.generated_at);
            protoReport->set_report_type(static_cast<int>(report.type));
            protoReport->set_total_assets(report.total_assets);
            protoReport->set_active_assets(report.active_assets);
            protoReport->set_inactive_assets(report.inactive_assets);
        }
        
        response->set_success(true);
        response->set_message("Reports retrieved successfully");
        
        return grpc::Status::OK;
        
    } catch (const std::exception& e) {
        spdlog::error("RPC: GetReports failed - {}", e.what());
        response->set_success(false);
        response->set_message(e.what());
        return grpc::Status(grpc::StatusCode::INTERNAL, e.what());
    }
}

grpc::Status BIReportServiceImpl::GetReport(
    grpc::ServerContext* context,
    const bi::GetReportRequest* request,
    bi::ReportResult* response) {
    
    spdlog::info("RPC: GetReport called - report_id={}", request->report_id());
    
    try {
        auto report = bi::ReportService::instance().getReportById(request->report_id());
        
        if (report.report_id.empty()) {
            return grpc::Status(grpc::StatusCode::NOT_FOUND, "Report not found");
        }
        
        response->set_report_id(report.report_id);
        response->set_title(report.title);
        response->set_generated_at(report.generated_at);
        response->set_report_type(static_cast<int>(report.type));
        response->set_total_assets(report.total_assets);
        response->set_active_assets(report.active_assets);
        response->set_inactive_assets(report.inactive_assets);
        
        for (const auto& section : report.sections) {
            auto protoSection = response->add_sections();
            protoSection->set_title(section.title);
            protoSection->set_chart_type(section.chart_type);
            
            for (const auto& dp : section.data_points) {
                auto protoDp = protoSection->add_data_points();
                protoDp->set_label(dp.label);
                protoDp->set_value(dp.value);
                protoDp->set_unit(dp.unit);
            }
        }
        
        return grpc::Status::OK;
        
    } catch (const std::exception& e) {
        spdlog::error("RPC: GetReport failed - {}", e.what());
        return grpc::Status(grpc::StatusCode::INTERNAL, e.what());
    }
}

grpc::Status BIReportServiceImpl::DeleteReport(
    grpc::ServerContext* context,
    const bi::DeleteReportRequest* request,
    bi::DeleteReportResponse* response) {
    
    spdlog::info("RPC: DeleteReport called - report_id={}", request->report_id());
    
    try {
        bi::ReportService::instance().deleteReport(request->report_id());
        
        response->set_success(true);
        response->set_message("Report deleted successfully");
        
        return grpc::Status::OK;
        
    } catch (const std::exception& e) {
        spdlog::error("RPC: DeleteReport failed - {}", e.what());
        response->set_success(false);
        response->set_message(e.what());
        return grpc::Status(grpc::StatusCode::INTERNAL, e.what());
    }
}

grpc::Status BIReportServiceImpl::AddTask(
    grpc::ServerContext* context,
    const bi::AddTaskRequest* request,
    bi::AddTaskResponse* response) {
    
    spdlog::info("RPC: AddTask called");
    
    try {
        bi::InternalScheduledTask task;
        task.task_id = "";
        task.name = request->task().name();
        task.report_type = static_cast<bi::ReportType>(request->task().report_type());
        task.cron_expression = request->task().cron_expression();
        task.format = static_cast<bi::ExportFormat>(request->task().export_format());
        task.enabled = request->task().enabled();
        
        for (int i = 0; i < request->task().recipients_size(); ++i) {
            task.recipients.push_back(request->task().recipients(i));
        }
        
        task.filter.start_date = request->task().filter().start_date();
        task.filter.end_date = request->task().filter().end_date();
        task.filter.limit = request->task().filter().limit();
        
        std::string taskId = bi::TaskScheduler::instance().addTask(task);
        
        response->set_task_id(taskId);
        response->set_success(true);
        response->set_message("Task added successfully");
        
        return grpc::Status::OK;
        
    } catch (const std::exception& e) {
        spdlog::error("RPC: AddTask failed - {}", e.what());
        response->set_success(false);
        response->set_message(e.what());
        return grpc::Status(grpc::StatusCode::INTERNAL, e.what());
    }
}

grpc::Status BIReportServiceImpl::GetTasks(
    grpc::ServerContext* context,
    const bi::GetTasksRequest* request,
    bi::GetTasksResponse* response) {
    
    spdlog::info("RPC: GetTasks called");
    
    try {
        auto tasks = bi::TaskScheduler::instance().getAllTasks();
        
        for (const auto& task : tasks) {
            auto protoTask = response->add_tasks();
            protoTask->set_task_id(task.task_id);
            protoTask->set_name(task.name);
            protoTask->set_report_type(static_cast<int>(task.report_type));
            protoTask->set_cron_expression(task.cron_expression);
            protoTask->set_export_format(static_cast<int>(task.format));
            protoTask->set_enabled(task.enabled);
            
            for (const auto& recipient : task.recipients) {
                protoTask->add_recipients(recipient);
            }
            
            auto* filter = protoTask->mutable_filter();
            filter->set_start_date(task.filter.start_date);
            filter->set_end_date(task.filter.end_date);
            filter->set_limit(task.filter.limit);
        }
        
        response->set_success(true);
        response->set_message("Tasks retrieved successfully");
        
        return grpc::Status::OK;
        
    } catch (const std::exception& e) {
        spdlog::error("RPC: GetTasks failed - {}", e.what());
        response->set_success(false);
        response->set_message(e.what());
        return grpc::Status(grpc::StatusCode::INTERNAL, e.what());
    }
}

grpc::Status BIReportServiceImpl::UpdateTask(
    grpc::ServerContext* context,
    const bi::UpdateTaskRequest* request,
    bi::UpdateTaskResponse* response) {
    
    spdlog::info("RPC: UpdateTask called - task_id={}", request->task_id());
    
    try {
        bi::InternalScheduledTask task;
        task.task_id = request->task_id();
        task.name = request->task().name();
        task.report_type = static_cast<bi::ReportType>(request->task().report_type());
        task.cron_expression = request->task().cron_expression();
        task.format = static_cast<bi::ExportFormat>(request->task().export_format());
        task.enabled = request->task().enabled();
        
        for (int i = 0; i < request->task().recipients_size(); ++i) {
            task.recipients.push_back(request->task().recipients(i));
        }
        
        bool success = bi::TaskScheduler::instance().updateTask(request->task_id(), task);
        
        response->set_success(success);
        response->set_message(success ? "Task updated successfully" : "Task not found");
        
        return grpc::Status::OK;
        
    } catch (const std::exception& e) {
        spdlog::error("RPC: UpdateTask failed - {}", e.what());
        response->set_success(false);
        response->set_message(e.what());
        return grpc::Status(grpc::StatusCode::INTERNAL, e.what());
    }
}

grpc::Status BIReportServiceImpl::DeleteTask(
    grpc::ServerContext* context,
    const bi::DeleteTaskRequest* request,
    bi::DeleteTaskResponse* response) {
    
    spdlog::info("RPC: DeleteTask called - task_id={}", request->task_id());
    
    try {
        bool success = bi::TaskScheduler::instance().removeTask(request->task_id());
        
        response->set_success(success);
        response->set_message(success ? "Task deleted successfully" : "Task not found");
        
        return grpc::Status::OK;
        
    } catch (const std::exception& e) {
        spdlog::error("RPC: DeleteTask failed - {}", e.what());
        response->set_success(false);
        response->set_message(e.what());
        return grpc::Status(grpc::StatusCode::INTERNAL, e.what());
    }
}

} // namespace rpc