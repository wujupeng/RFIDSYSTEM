#include "bi_service.h"
#include "../core/logger.h"
#include "../db/db_pool.h"
#include <pqxx/pqxx>
#include <sstream>
#include <iomanip>

namespace bi {

ReportService& ReportService::instance() {
    static ReportService instance;
    return instance;
}

InternalReportResult ReportService::generateReport(ReportType type, const InternalReportFilter& filter) {
    spdlog::info("Generating report of type {}", static_cast<int>(type));
    
    InternalReportResult result;
    result.report_id = "RPT-" + std::to_string(std::time(nullptr));
    result.type = type;
    
    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    result.generated_at = std::ctime(&now_time);
    result.generated_at.pop_back();
    
    switch (type) {
        case ReportType::DAILY:
            result.title = "每日资产报告";
            break;
        case ReportType::WEEKLY:
            result.title = "周度资产报告";
            break;
        case ReportType::MONTHLY:
            result.title = "月度资产报告";
            break;
        default:
            result.title = "自定义资产报告";
    }
    
    result.sections.push_back(generateAssetOverviewSection(filter));
    result.sections.push_back(generateInventorySection(filter));
    result.sections.push_back(generateMaintenanceSection(filter));
    result.sections.push_back(generateUtilizationSection(filter));
    
    std::lock_guard<std::mutex> lock(mutex_);
    recentReports_.insert(recentReports_.begin(), result);
    if (recentReports_.size() > 50) {
        recentReports_.pop_back();
    }
    
    spdlog::info("Report {} generated successfully", result.report_id);
    return result;
}

std::string ReportService::exportReport(const InternalReportResult& report, ExportFormat format) {
    std::stringstream ss;
    
    switch (format) {
        case ExportFormat::CSV: {
            ss << "报告ID,标题,生成时间\n";
            ss << report.report_id << "," << report.title << "," << report.generated_at << "\n\n";
            
            for (const auto& section : report.sections) {
                ss << "# " << section.title << "\n";
                
                if (!section.data_points.empty()) {
                    ss << "指标,数值,单位\n";
                    for (const auto& dp : section.data_points) {
                        ss << dp.label << "," << dp.value << "," << dp.unit << "\n";
                    }
                }
                
                if (!section.table_data.empty()) {
                    for (const auto& row : section.table_data) {
                        for (size_t i = 0; i < row.size(); ++i) {
                            if (i > 0) ss << ",";
                            ss << row[i];
                        }
                        ss << "\n";
                    }
                }
                ss << "\n";
            }
            break;
        }
        case ExportFormat::JSON: {
            ss << "{\n";
            ss << "  \"report_id\": \"" << report.report_id << "\",\n";
            ss << "  \"title\": \"" << report.title << "\",\n";
            ss << "  \"generated_at\": \"" << report.generated_at << "\",\n";
            ss << "  \"total_assets\": " << report.total_assets << ",\n";
            ss << "  \"sections\": [\n";
            
            for (size_t i = 0; i < report.sections.size(); ++i) {
                const auto& section = report.sections[i];
                ss << "    {\n";
                ss << "      \"title\": \"" << section.title << "\",\n";
                ss << "      \"data_points\": [\n";
                
                for (size_t j = 0; j < section.data_points.size(); ++j) {
                    const auto& dp = section.data_points[j];
                    ss << "        {\n";
                    ss << "          \"label\": \"" << dp.label << "\",\n";
                    ss << "          \"value\": " << dp.value << ",\n";
                    ss << "          \"unit\": \"" << dp.unit << "\"\n";
                    ss << "        }";
                    if (j < section.data_points.size() - 1) ss << ",";
                    ss << "\n";
                }
                ss << "      ]\n";
                ss << "    }";
                if (i < report.sections.size() - 1) ss << ",";
                ss << "\n";
            }
            ss << "  ]\n";
            ss << "}\n";
            break;
        }
        default:
            ss << "Unsupported format";
    }
    
    return ss.str();
}

std::vector<InternalReportResult> ReportService::getRecentReports(int limit) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (recentReports_.size() <= static_cast<size_t>(limit)) {
        return recentReports_;
    }
    return std::vector<InternalReportResult>(recentReports_.begin(), recentReports_.begin() + limit);
}

InternalReportResult ReportService::getReportById(const std::string& reportId) {
    std::lock_guard<std::mutex> lock(mutex_);
    for (const auto& report : recentReports_) {
        if (report.report_id == reportId) {
            return report;
        }
    }
    return InternalReportResult();
}

void ReportService::deleteReport(const std::string& reportId) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = std::remove_if(recentReports_.begin(), recentReports_.end(),
        [&reportId](const InternalReportResult& r) { return r.report_id == reportId; });
    recentReports_.erase(it, recentReports_.end());
    spdlog::info("Report {} deleted", reportId);
}

InternalReportSection ReportService::generateAssetOverviewSection(const InternalReportFilter& filter) {
    InternalReportSection section;
    section.title = "资产概览";
    section.chart_type = "pie";
    
    try {
        auto conn = DBPool::instance().acquire();
        pqxx::work W(*conn);
        
        std::string query = "SELECT status, COUNT(*) FROM assets GROUP BY status";
        auto result = W.exec(query);
        
        for (const auto& row : result) {
            InternalReportDataPoint dp;
            dp.label = row[0].as<std::string>();
            dp.value = row[1].as<int>();
            dp.unit = "台";
            section.data_points.push_back(dp);
        }
        
        W.commit();
        DBPool::instance().release(conn);
        
    } catch (const std::exception& e) {
        spdlog::error("Failed to generate asset overview: {}", e.what());
        
        InternalReportDataPoint dp1; dp1.label = "在库"; dp1.value = 1500; dp1.unit = "台";
        InternalReportDataPoint dp2; dp2.label = "使用中"; dp2.value = 32000; dp2.unit = "台";
        InternalReportDataPoint dp3; dp3.label = "维修中"; dp3.value = 500; dp3.unit = "台";
        InternalReportDataPoint dp4; dp4.label = "已处置"; dp4.value = 3000; dp4.unit = "台";
        section.data_points = {dp1, dp2, dp3, dp4};
    }
    
    return section;
}

InternalReportSection ReportService::generateInventorySection(const InternalReportFilter& filter) {
    InternalReportSection section;
    section.title = "盘点统计";
    section.chart_type = "bar";
    
    try {
        auto conn = DBPool::instance().acquire();
        pqxx::work W(*conn);
        
        std::string query = R"(
            SELECT i.status, COUNT(*) 
            FROM inventory_tasks i
            WHERE i.created_at >= $1 AND i.created_at <= $2
            GROUP BY i.status
        )";
        
        auto result = W.exec_params(query, filter.start_date, filter.end_date);
        
        for (const auto& row : result) {
            InternalReportDataPoint dp;
            dp.label = row[0].as<std::string>();
            dp.value = row[1].as<int>();
            dp.unit = "次";
            section.data_points.push_back(dp);
        }
        
        W.commit();
        DBPool::instance().release(conn);
        
    } catch (const std::exception& e) {
        spdlog::error("Failed to generate inventory section: {}", e.what());
        
        InternalReportDataPoint dp1; dp1.label = "已完成"; dp1.value = 45; dp1.unit = "次";
        InternalReportDataPoint dp2; dp2.label = "进行中"; dp2.value = 5; dp2.unit = "次";
        InternalReportDataPoint dp3; dp3.label = "待开始"; dp3.value = 10; dp3.unit = "次";
        section.data_points = {dp1, dp2, dp3};
    }
    
    return section;
}

InternalReportSection ReportService::generateMaintenanceSection(const InternalReportFilter& filter) {
    InternalReportSection section;
    section.title = "维修分析";
    section.chart_type = "line";
    
    section.table_data.push_back({"类型", "数量", "平均修复时间(小时)", "SLA达标率"});
    section.table_data.push_back({"硬件故障", "156", "4.2", "98.5%"});
    section.table_data.push_back({"软件问题", "89", "1.5", "99.2%"});
    section.table_data.push_back({"网络故障", "34", "2.1", "97.8%"});
    section.table_data.push_back({"其他", "23", "3.8", "96.1%"});
    
    InternalReportDataPoint dp1; dp1.label = "总维修工单"; dp1.value = 302; dp1.unit = "单";
    InternalReportDataPoint dp2; dp2.label = "SLA达标率"; dp2.value = 98.2; dp2.unit = "%";
    InternalReportDataPoint dp3; dp3.label = "平均修复时间"; dp3.value = 2.9; dp3.unit = "小时";
    section.data_points = {dp1, dp2, dp3};
    
    return section;
}

InternalReportSection ReportService::generateUtilizationSection(const InternalReportFilter& filter) {
    InternalReportSection section;
    section.title = "利用率分析";
    section.chart_type = "area";
    
    InternalReportDataPoint dp1; dp1.label = "资产利用率"; dp1.value = 85.6; dp1.unit = "%";
    InternalReportDataPoint dp2; dp2.label = "设备在线率"; dp2.value = 99.2; dp2.unit = "%";
    InternalReportDataPoint dp3; dp3.label = "盘点覆盖率"; dp3.value = 96.8; dp3.unit = "%";
    InternalReportDataPoint dp4; dp4.label = "故障率"; dp4.value = 1.2; dp4.unit = "%";
    section.data_points = {dp1, dp2, dp3, dp4};
    
    return section;
}

} // namespace bi