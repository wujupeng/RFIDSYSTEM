#pragma once
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <mutex>
#include <chrono>
#include <functional>

namespace bi {

enum class ReportType {
    DAILY,
    WEEKLY,
    MONTHLY,
    CUSTOM
};

enum class ExportFormat {
    CSV,
    EXCEL,
    PDF,
    JSON
};

struct InternalReportFilter {
    std::string start_date;
    std::string end_date;
    std::vector<std::string> asset_categories;
    std::vector<std::string> locations;
    std::vector<std::string> statuses;
    int limit = 1000;
};

struct InternalReportDataPoint {
    std::string label;
    double value;
    std::string unit;
    std::map<std::string, std::string> metadata;
};

struct InternalReportSection {
    std::string title;
    std::vector<InternalReportDataPoint> data_points;
    std::vector<std::vector<std::string>> table_data;
    std::string chart_type;
};

struct InternalReportResult {
    std::string report_id;
    std::string title;
    std::string generated_at;
    ReportType type;
    std::vector<InternalReportSection> sections;
    int total_assets;
    int active_assets;
    int inactive_assets;
};

struct InternalScheduledTask {
    std::string task_id;
    std::string name;
    ReportType report_type;
    std::string cron_expression;
    ExportFormat format;
    bool enabled;
    std::vector<std::string> recipients;
    InternalReportFilter filter;
};

class ReportService {
public:
    static ReportService& instance();

    InternalReportResult generateReport(ReportType type, const InternalReportFilter& filter);
    std::string exportReport(const InternalReportResult& report, ExportFormat format);
    
    std::vector<InternalReportResult> getRecentReports(int limit = 10);
    InternalReportResult getReportById(const std::string& reportId);
    void deleteReport(const std::string& reportId);

private:
    ReportService() = default;
    ReportService(const ReportService&) = delete;
    ReportService& operator=(const ReportService&) = delete;

    InternalReportSection generateAssetOverviewSection(const InternalReportFilter& filter);
    InternalReportSection generateInventorySection(const InternalReportFilter& filter);
    InternalReportSection generateMaintenanceSection(const InternalReportFilter& filter);
    InternalReportSection generateUtilizationSection(const InternalReportFilter& filter);

    std::vector<InternalReportResult> recentReports_;
    mutable std::mutex mutex_;
};

} // namespace bi