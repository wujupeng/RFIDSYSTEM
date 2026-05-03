#pragma once

#include <string>
#include <vector>
#include <memory>
#include <functional>

namespace audit {

struct AuditRecord {
    int id;
    int user_id;
    std::string action;
    std::string target_type;
    int target_id;
    std::string result;
    std::string ip_address;
    std::string details;
    std::string created_at;
};

class AuditService {
public:
    static AuditService& instance();

    bool logAction(int userId, const std::string& action, 
                   const std::string& targetType, int targetId,
                   const std::string& result, const std::string& ipAddress = "",
                   const std::string& details = "");

    std::vector<AuditRecord> queryLogs(int userId, const std::string& action,
                                       const std::string& targetType, int targetId,
                                       const std::string& startTime, const std::string& endTime);

    std::vector<AuditRecord> getRecentLogs(int limit = 100);

    int getActionCount(const std::string& action, const std::string& result,
                       const std::string& startTime, const std::string& endTime);

private:
    AuditService();
    AuditService(const AuditService&) = delete;
    AuditService& operator=(const AuditService&) = delete;
};

#define AUDIT_LOG(userId, action, targetType, targetId, result, ip, details) \
    audit::AuditService::instance().logAction(userId, action, targetType, targetId, result, ip, details)

} // namespace audit