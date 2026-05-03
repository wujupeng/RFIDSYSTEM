#include "inventory_service.h"
#include "../db/db_pool.h"
#include "../core/logger.h"
#include "../core/epc_validator.h"
#include "../core/idempotency_manager.h"
#include "reconciliation_service.h"
#include <unordered_set>
#include <sstream>

InventoryService& InventoryService::instance() {
    static InventoryService instance;
    return instance;
}

int InventoryService::startTask(const std::string& taskName, const std::string& location, const std::string& operatorName) {
    spdlog::info("Starting inventory task: name={}, location={}, operator={}", taskName, location, operatorName);

    auto conn = DBPool::instance().acquire();
    pqxx::work W(*conn);

    pqxx::result R = W.exec(
        "INSERT INTO inventory_tasks(task_name, location, operator) VALUES(" +
        W.quote(taskName) + ", " +
        W.quote(location) + ", " +
        W.quote(operatorName) + ") RETURNING id"
    );

    int taskId = R[0][0].as<int>();
    W.commit();

    spdlog::info("Inventory task created: id={}", taskId);
    DBPool::instance().release(conn);
    return taskId;
}

bool InventoryService::updateTask(int taskId, int scannedCount, int foundCount, int missingCount, int extraCount) {
    auto conn = DBPool::instance().acquire();
    pqxx::work W(*conn);

    W.exec(
        "UPDATE inventory_tasks SET "
        "scanned_count = " + W.to_string(scannedCount) + ", " +
        "found_count = " + W.to_string(foundCount) + ", " +
        "missing_count = " + W.to_string(missingCount) + ", " +
        "extra_count = " + W.to_string(extraCount) + ", " +
        "status = 'IN_PROGRESS' "
        "WHERE id = " + W.to_string(taskId)
    );

    W.commit();
    DBPool::instance().release(conn);
    return true;
}

bool InventoryService::completeTask(int taskId) {
    auto conn = DBPool::instance().acquire();
    pqxx::work W(*conn);

    W.exec(
        "UPDATE inventory_tasks SET "
        "status = 'COMPLETED', "
        "completed_at = CURRENT_TIMESTAMP "
        "WHERE id = " + W.to_string(taskId)
    );

    W.commit();
    DBPool::instance().release(conn);
    spdlog::info("Inventory task completed: id={}", taskId);
    return true;
}

InventoryTask InventoryService::getTask(int taskId) {
    auto conn = DBPool::instance().acquire();
    pqxx::work W(*conn);

    pqxx::result R = W.exec(
        "SELECT id, task_name, status, scanned_count, found_count, "
        "missing_count, extra_count, location, operator, created_at, completed_at "
        "FROM inventory_tasks WHERE id = " + W.to_string(taskId)
    );

    InventoryTask task{};
    if (!R.empty()) {
        task.id = R[0][0].as<int>();
        task.task_name = R[0][1].as<std::string>();
        task.status = R[0][2].as<std::string>();
        task.scanned_count = R[0][3].as<int>();
        task.found_count = R[0][4].as<int>();
        task.missing_count = R[0][5].as<int>();
        task.extra_count = R[0][6].as<int>();
        task.location = R[0][7].as<std::string>();
        task.operator_name = R[0][8].as<std::string>();
    }

    DBPool::instance().release(conn);
    return task;
}

std::vector<InventoryService::ScanResultDetail> InventoryService::scanEPCsWithDetails(const std::vector<std::string>& epcs, int taskId) {
    spdlog::info("Scanning {} EPCs, task_id={}", epcs.size(), taskId);

    std::vector<std::string> validEPCs = data::EPCValidator::filterValidEPCs(epcs);
    std::vector<std::string> invalidEPCs = data::EPCValidator::filterInvalidEPCs(epcs);

    for (const auto& epc : invalidEPCs) {
        data::ReconciliationService::instance().recordDirtyData(epc, "Invalid EPC format");
    }

    spdlog::debug("Valid EPCs: {}, Invalid EPCs: {}", validEPCs.size(), invalidEPCs.size());

    std::unordered_set<std::string> epcSet(validEPCs.begin(), validEPCs.end());
    std::vector<std::string> uniqueEpcs(epcSet.begin(), epcSet.end());

    std::vector<std::string> toProcess;
    if (taskId > 0) {
        auto& idempotency = data::IdempotencyManager::instance();
        for (const auto& epc : uniqueEpcs) {
            if (!idempotency.isProcessed(taskId, epc)) {
                toProcess.push_back(epc);
                idempotency.markProcessed(taskId, epc);
            }
        }
        spdlog::debug("After idempotency check: processing {}, skipped {}", toProcess.size(), uniqueEpcs.size() - toProcess.size());
    } else {
        toProcess = uniqueEpcs;
    }

    auto conn = DBPool::instance().acquire();
    pqxx::work W(*conn);

    if (toProcess.empty()) {
        DBPool::instance().release(conn);
        return {};
    }

    std::stringstream ss;
    ss << "SELECT rfid_epc, id, asset_code, name, status FROM assets WHERE rfid_epc IN (";
    for (size_t i = 0; i < toProcess.size(); ++i) {
        if (i > 0) ss << ",";
        ss << W.quote(toProcess[i]);
    }
    ss << ")";

    pqxx::result R = W.exec(ss.str());

    std::vector<ScanResultDetail> foundDetails;
    std::unordered_set<std::string> foundEPCs;

    for (const auto& row : R) {
        ScanResultDetail detail;
        detail.epc = row[0].as<std::string>();
        detail.asset_id = row[1].as<int>();
        detail.asset_code = row[2].as<std::string>();
        detail.asset_name = row[3].as<std::string>();
        detail.status = row[4].as<std::string>();
        foundDetails.push_back(detail);
        foundEPCs.insert(detail.epc);
    }

    if (taskId > 0) {
        for (const auto& detail : foundDetails) {
            try {
                W.exec(
                    "INSERT INTO scan_records(task_id, epc, first_scan_time, last_scan_time) VALUES(" +
                    W.to_string(taskId) + ", " +
                    W.quote(detail.epc) + ", NOW(), NOW()) "
                    "ON CONFLICT (task_id, epc) DO UPDATE SET scan_count = scan_records.scan_count + 1, last_scan_time = NOW()"
                );
            } catch (const std::exception& e) {
                spdlog::warn("Failed to insert scan record: {}", e.what());
            }
        }
        W.commit();
    }

    DBPool::instance().release(conn);
    spdlog::info("Scan completed: found={}", foundDetails.size());
    return foundDetails;
}

InventoryResult InventoryService::scanEPCs(const std::vector<std::string>& epcs, int taskId) {
    std::vector<std::string> validEPCs = data::EPCValidator::filterValidEPCs(epcs);
    std::vector<std::string> invalidEPCs = data::EPCValidator::filterInvalidEPCs(epcs);

    for (const auto& epc : invalidEPCs) {
        data::ReconciliationService::instance().recordDirtyData(epc, "Invalid EPC format");
    }

    std::unordered_set<std::string> epcSet(validEPCs.begin(), validEPCs.end());
    std::vector<std::string> uniqueEpcs(epcSet.begin(), epcSet.end());

    std::vector<std::string> toProcess;
    if (taskId > 0) {
        auto& idempotency = data::IdempotencyManager::instance();
        for (const auto& epc : uniqueEpcs) {
            if (!idempotency.isProcessed(taskId, epc)) {
                toProcess.push_back(epc);
                idempotency.markProcessed(taskId, epc);
            }
        }
    } else {
        toProcess = uniqueEpcs;
    }

    auto conn = DBPool::instance().acquire();
    pqxx::work W(*conn);

    std::stringstream ss;
    ss << "SELECT rfid_epc FROM assets WHERE status != 'SCRAPPED'";
    pqxx::result allAssets = W.exec(ss.str());

    std::unordered_set<std::string> allAssetEPCs;
    for (const auto& row : allAssets) {
        allAssetEPCs.insert(row[0].as<std::string>());
    }

    std::stringstream foundSS;
    foundSS << "SELECT rfid_epc FROM assets WHERE rfid_epc IN (";
    for (size_t i = 0; i < toProcess.size(); ++i) {
        if (i > 0) foundSS << ",";
        foundSS << W.quote(toProcess[i]);
    }
    foundSS << ")";
    pqxx::result foundResult = W.exec(foundSS.str());

    std::unordered_set<std::string> foundEPCs;
    for (const auto& row : foundResult) {
        foundEPCs.insert(row[0].as<std::string>());
    }

    InventoryResult result;

    for (const auto& epc : toProcess) {
        if (foundEPCs.count(epc)) {
            result.found.push_back(epc);
        } else {
            result.extra.push_back(epc);
        }
    }

    for (const auto& assetEpc : allAssetEPCs) {
        if (!foundEPCs.count(assetEpc)) {
            result.missing.push_back(assetEpc);
        }
    }

    if (taskId > 0) {
        for (const auto& epc : result.found) {
            try {
                W.exec(
                    "INSERT INTO scan_records(task_id, epc, first_scan_time, last_scan_time) VALUES(" +
                    W.to_string(taskId) + ", " +
                    W.quote(epc) + ", NOW(), NOW()) "
                    "ON CONFLICT (task_id, epc) DO UPDATE SET scan_count = scan_records.scan_count + 1, last_scan_time = NOW()"
                );
            } catch (const std::exception& e) {
                spdlog::warn("Failed to insert scan record: {}", e.what());
            }
        }
        updateTask(taskId, toProcess.size(), result.found.size(), result.missing.size(), result.extra.size());
        W.commit();
    }

    DBPool::instance().release(conn);

    spdlog::info("Inventory scan result: valid={}, found={}, missing={}, extra={}, invalid={}",
                 toProcess.size(), result.found.size(), result.missing.size(), result.extra.size(), invalidEPCs.size());

    return result;
}