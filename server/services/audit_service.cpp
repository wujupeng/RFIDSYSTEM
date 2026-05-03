#include "audit_service.h"
#include "../db/db_pool.h"
#include "../core/logger.h"
#include <pqxx/pqxx>

namespace audit {

AuditService::AuditService() {
    spdlog::info("AuditService initialized");
}

AuditService& AuditService::instance() {
    static AuditService instance;
    return instance;
}

bool AuditService::logAction(int userId, const std::string& action,
                             const std::string& targetType, int targetId,
                             const std::string& result, const std::string& ipAddress,
                             const std::string& details) {
    try {
        auto conn = DBPool::instance().getConnection();
        pqxx::work txn(*conn);
        
        std::string query = R"(
            INSERT INTO audit_logs (user_id, action, target_type, target_id, result, ip_address, details)
            VALUES ($1, $2, $3, $4, $5, $6, $7)
        )";
        
        txn.exec_params(query, userId, action, targetType, targetId, result, ipAddress, details);
        txn.commit();
        
        spdlog::debug("Audit log written: user={}, action={}, target={}:{}", 
                      userId, action, targetType, targetId);
        return true;
    } catch (const std::exception& e) {
        spdlog::error("Failed to write audit log: {}", e.what());
        return false;
    }
}

std::vector<AuditRecord> AuditService::queryLogs(int userId, const std::string& action,
                                                 const std::string& targetType, int targetId,
                                                 const std::string& startTime, const std::string& endTime) {
    std::vector<AuditRecord> records;
    
    try {
        auto conn = DBPool::instance().getConnection();
        pqxx::work txn(*conn);
        
        std::string query = "SELECT id, user_id, action, target_type, target_id, result, ip_address, details, created_at FROM audit_logs WHERE 1=1";
        std::vector<std::string> params;
        int paramIndex = 1;
        
        if (userId > 0) {
            query += " AND user_id = $" + std::to_string(paramIndex++);
            params.push_back(std::to_string(userId));
        }
        if (!action.empty()) {
            query += " AND action = $" + std::to_string(paramIndex++);
            params.push_back(action);
        }
        if (!targetType.empty()) {
            query += " AND target_type = $" + std::to_string(paramIndex++);
            params.push_back(targetType);
        }
        if (targetId > 0) {
            query += " AND target_id = $" + std::to_string(paramIndex++);
            params.push_back(std::to_string(targetId));
        }
        if (!startTime.empty()) {
            query += " AND created_at >= $" + std::to_string(paramIndex++);
            params.push_back(startTime);
        }
        if (!endTime.empty()) {
            query += " AND created_at <= $" + std::to_string(paramIndex++);
            params.push_back(endTime);
        }
        
        query += " ORDER BY created_at DESC LIMIT 1000";
        
        pqxx::result res = txn.exec(query);
        
        for (const auto& row : res) {
            AuditRecord record;
            record.id = row["id"].as<int>();
            record.user_id = row["user_id"].as<int>();
            record.action = row["action"].as<std::string>();
            record.target_type = row["target_type"].as<std::string>();
            record.target_id = row["target_id"].as<int>();
            record.result = row["result"].as<std::string>();
            record.ip_address = row["ip_address"].as<std::string>();
            record.details = row["details"].as<std::string>();
            record.created_at = row["created_at"].as<std::string>();
            records.push_back(record);
        }
    } catch (const std::exception& e) {
        spdlog::error("Failed to query audit logs: {}", e.what());
    }
    
    return records;
}

std::vector<AuditRecord> AuditService::getRecentLogs(int limit) {
    std::vector<AuditRecord> records;
    
    try {
        auto conn = DBPool::instance().getConnection();
        pqxx::work txn(*conn);
        
        std::string query = "SELECT id, user_id, action, target_type, target_id, result, ip_address, details, created_at "
                           "FROM audit_logs ORDER BY created_at DESC LIMIT $1";
        
        pqxx::result res = txn.exec_params(query, limit);
        
        for (const auto& row : res) {
            AuditRecord record;
            record.id = row["id"].as<int>();
            record.user_id = row["user_id"].as<int>();
            record.action = row["action"].as<std::string>();
            record.target_type = row["target_type"].as<std::string>();
            record.target_id = row["target_id"].as<int>();
            record.result = row["result"].as<std::string>();
            record.ip_address = row["ip_address"].as<std::string>();
            record.details = row["details"].as<std::string>();
            record.created_at = row["created_at"].as<std::string>();
            records.push_back(record);
        }
    } catch (const std::exception& e) {
        spdlog::error("Failed to get recent audit logs: {}", e.what());
    }
    
    return records;
}

int AuditService::getActionCount(const std::string& action, const std::string& result,
                                  const std::string& startTime, const std::string& endTime) {
    try {
        auto conn = DBPool::instance().getConnection();
        pqxx::work txn(*conn);
        
        std::string query = "SELECT COUNT(*) FROM audit_logs WHERE 1=1";
        std::vector<std::string> params;
        int paramIndex = 1;
        
        if (!action.empty()) {
            query += " AND action = $" + std::to_string(paramIndex++);
            params.push_back(action);
        }
        if (!result.empty()) {
            query += " AND result = $" + std::to_string(paramIndex++);
            params.push_back(result);
        }
        if (!startTime.empty()) {
            query += " AND created_at >= $" + std::to_string(paramIndex++);
            params.push_back(startTime);
        }
        if (!endTime.empty()) {
            query += " AND created_at <= $" + std::to_string(paramIndex++);
            params.push_back(endTime);
        }
        
        pqxx::result res = txn.exec(query);
        return res[0][0].as<int>();
    } catch (const std::exception& e) {
        spdlog::error("Failed to get action count: {}", e.what());
        return 0;
    }
}

} // namespace audit