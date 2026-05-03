#include "reconciliation_service.h"
#include "../db/db_pool.h"
#include "../core/logger.h"
#include <sstream>
#include <algorithm>

namespace data {

ReconciliationService& ReconciliationService::instance() {
    static ReconciliationService instance;
    return instance;
}

ReconciliationService::ReconciliationService() {
    spdlog::info("ReconciliationService initialized");
}

ReconciliationResult ReconciliationService::reconcile(int taskId, const std::vector<std::string>& scannedEPCs) {
    ReconciliationResult result;
    
    try {
        auto conn = DBPool::instance().getConnection();
        
        std::string query = R"(
            SELECT rfid_epc FROM assets 
            WHERE location = (SELECT location FROM inventory_tasks WHERE id = $1)
            AND status != 'SCRAPPED'
        )";
        
        auto resultSet = conn->exec_params(query, taskId);
        
        std::unordered_set<std::string> expectedEPCs;
        for (const auto& row : resultSet) {
            expectedEPCs.insert(row["rfid_epc"].as<std::string>());
        }
        
        std::unordered_set<std::string> scannedSet(scannedEPCs.begin(), scannedEPCs.end());
        
        for (const auto& epc : expectedEPCs) {
            if (scannedSet.count(epc)) {
                result.found_epcs.push_back(epc);
            } else {
                result.missing_epcs.push_back(epc);
            }
        }
        
        for (const auto& epc : scannedSet) {
            if (!expectedEPCs.count(epc)) {
                result.extra_epcs.push_back(epc);
            }
        }
        
        result.total_expected = expectedEPCs.size();
        result.total_found = result.found_epcs.size();
        result.total_missing = result.missing_epcs.size();
        result.total_extra = result.extra_epcs.size();
        
        if (result.total_expected > 0) {
            result.accuracy_rate = static_cast<double>(result.total_found) / result.total_expected;
            result.loss_rate = static_cast<double>(result.total_missing) / result.total_expected;
        } else {
            result.accuracy_rate = 0.0;
            result.loss_rate = 0.0;
        }
        
        result.profit_rate = result.total_expected > 0 
            ? static_cast<double>(result.total_extra) / result.total_expected 
            : 0.0;
        
        spdlog::info("Reconciliation completed - task={}, expected={}, found={}, missing={}, extra={}, accuracy={:.2%}",
            taskId, result.total_expected, result.total_found, 
            result.total_missing, result.total_extra, result.accuracy_rate);
            
    } catch (const std::exception& e) {
        spdlog::error("Reconciliation failed - {}", e.what());
    }
    
    return result;
}

void ReconciliationService::recordDirtyData(const std::string& rawEPC, const std::string& reason, const std::string& sourceReader) {
    try {
        auto conn = DBPool::instance().getConnection();
        
        std::string query = R"(
            INSERT INTO dirty_epcs (raw_epc, reason, source_reader, timestamp)
            VALUES ($1, $2, $3, NOW())
        )";
        
        conn->exec_params(query, rawEPC, reason, sourceReader);
        
        spdlog::warn("Dirty EPC recorded - epc={}, reason={}, source={}", rawEPC, reason, sourceReader);
        
    } catch (const std::exception& e) {
        spdlog::error("Failed to record dirty data - {}", e.what());
    }
}

std::vector<DirtyDataRecord> ReconciliationService::getDirtyData(int limit) {
    std::vector<DirtyDataRecord> records;
    
    try {
        auto conn = DBPool::instance().getConnection();
        
        std::string query = "SELECT id, raw_epc, reason, source_reader, EXTRACT(EPOCH FROM timestamp)::bigint as ts FROM dirty_epcs ORDER BY timestamp DESC LIMIT $1";
        
        auto resultSet = conn->exec_params(query, limit);
        
        for (const auto& row : resultSet) {
            DirtyDataRecord record;
            record.id = row["id"].as<int>();
            record.raw_epc = row["raw_epc"].as<std::string>();
            record.reason = row["reason"].as<std::string>();
            record.source_reader = row["source_reader"].as<std::string>();
            record.timestamp = row["ts"].as<int64_t>();
            records.push_back(record);
        }
        
    } catch (const std::exception& e) {
        spdlog::error("Failed to get dirty data - {}", e.what());
    }
    
    return records;
}

int ReconciliationService::getDirtyDataCount() {
    try {
        auto conn = DBPool::instance().getConnection();
        auto result = conn->exec("SELECT COUNT(*) FROM dirty_epcs");
        return result[0][0].as<int>();
    } catch (const std::exception& e) {
        spdlog::error("Failed to get dirty data count - {}", e.what());
        return 0;
    }
}

void ReconciliationService::clearDirtyData() {
    try {
        auto conn = DBPool::instance().getConnection();
        conn->exec("DELETE FROM dirty_epcs");
        spdlog::info("Dirty data cleared");
    } catch (const std::exception& e) {
        spdlog::error("Failed to clear dirty data - {}", e.what());
    }
}

} // namespace data