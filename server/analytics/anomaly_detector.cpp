#include "anomaly_detector.h"
#include "../db/db_pool.h"
#include "../core/logger.h"
#include "../core/alert.h"
#include <sstream>
#include <algorithm>

namespace analytics {

AnomalyDetector& AnomalyDetector::instance() {
    static AnomalyDetector instance;
    return instance;
}

AnomalyDetector::AnomalyDetector() {
    spdlog::info("AnomalyDetector initialized");
}

void AnomalyDetector::setMissingThreshold(int minutes) {
    missingThresholdMinutes_ = minutes;
}

void AnomalyDetector::setHighFrequencyThreshold(int moves, int minutes) {
    highFrequencyMoveCount_ = moves;
    highFrequencyTimeWindowMinutes_ = minutes;
}

void AnomalyDetector::setValidLocations(const std::vector<std::string>& locations) {
    validLocations_ = locations;
}

void AnomalyDetector::addIllegalLocation(const std::string& location) {
    if (std::find(illegalLocations_.begin(), illegalLocations_.end(), location) == illegalLocations_.end()) {
        illegalLocations_.push_back(location);
    }
}

std::string AnomalyDetector::typeToString(AnomalyType type) {
    switch (type) {
        case AnomalyType::ASSET_MISSING: return "ASSET_MISSING";
        case AnomalyType::HIGH_FREQUENCY_MOVE: return "HIGH_FREQUENCY_MOVE";
        case AnomalyType::ILLEGAL_LOCATION: return "ILLEGAL_LOCATION";
        case AnomalyType::STALE_DATA: return "STALE_DATA";
        case AnomalyType::UNEXPECTED_READER: return "UNEXPECTED_READER";
        case AnomalyType::DUPLICATE_EPC: return "DUPLICATE_EPC";
        default: return "UNKNOWN";
    }
}

std::string AnomalyDetector::severityToString(AnomalySeverity severity) {
    switch (severity) {
        case AnomalySeverity::INFO: return "INFO";
        case AnomalySeverity::WARNING: return "WARNING";
        case AnomalySeverity::ERROR: return "ERROR";
        case AnomalySeverity::CRITICAL: return "CRITICAL";
        default: return "UNKNOWN";
    }
}

void AnomalyDetector::recordAnomaly(int assetId, const std::string& epc, AnomalyType type,
                                   AnomalySeverity severity, const std::string& message,
                                   const std::string& location) {
    try {
        auto conn = DBPool::instance().acquire();
        pqxx::work W(*conn);
        
        pqxx::result existing = W.exec(
            "SELECT id FROM anomaly_events WHERE asset_id = " + std::to_string(assetId) + " "
            "AND anomaly_type = " + W.quote(typeToString(type)) + " AND is_resolved = FALSE"
        );
        
        if (!existing.empty()) {
            W.exec(
                "UPDATE anomaly_events SET detected_at = NOW(), message = " + W.quote(message) + " "
                "WHERE id = " + std::to_string(existing[0][0].as<int>())
            );
        } else {
            W.exec(
                "INSERT INTO anomaly_events "
                "(asset_id, epc, anomaly_type, severity, message, location) "
                "VALUES(" +
                std::to_string(assetId) + ", " +
                W.quote(epc) + ", " +
                W.quote(typeToString(type)) + ", " +
                W.quote(severityToString(severity)) + ", " +
                W.quote(message) + ", " +
                W.quote(location) + ")"
            );
        }
        
        W.commit();
        DBPool::instance().release(conn);
        
        if (severity >= AnomalySeverity::WARNING) {
            ALERT_WARN(typeToString(type), message);
        }
        
        spdlog::warn("Anomaly detected: type={}, asset={}, epc={}, message={}", 
                     typeToString(type), assetId, epc, message);
        
    } catch (const std::exception& e) {
        spdlog::error("Failed to record anomaly: {}", e.what());
    }
}

void AnomalyDetector::scanForAnomalies() {
    spdlog::debug("Scanning for anomalies...");
    
    try {
        auto conn = DBPool::instance().acquire();
        pqxx::work W(*conn);
        
        pqxx::result missingResult = W.exec(
            "SELECT a.id, a.rfid_epc, a.location "
            "FROM assets a "
            "LEFT JOIN scan_records sr ON a.id = sr.asset_id "
            "WHERE a.status != 'SCRAPPED' "
            "AND (sr.last_scan_time IS NULL OR sr.last_scan_time < NOW() - INTERVAL '" + 
            std::to_string(missingThresholdMinutes_) + " minutes')"
        );
        
        for (const auto& row : missingResult) {
            int assetId = row[0].as<int>();
            std::string epc = row[1].as<std::string>();
            std::string location = row[2].as<std::string>();
            
            std::string message = "Asset not scanned for " + std::to_string(missingThresholdMinutes_) + " minutes";
            recordAnomaly(assetId, epc, AnomalyType::ASSET_MISSING, AnomalySeverity::WARNING, message, location);
        }
        
        pqxx::result freqResult = W.exec(
            "SELECT asset_id, epc, COUNT(*) as move_count "
            "FROM asset_location_history "
            "WHERE entry_time >= NOW() - INTERVAL '" + std::to_string(highFrequencyTimeWindowMinutes_) + " minutes' "
            "GROUP BY asset_id, epc HAVING COUNT(*) > " + std::to_string(highFrequencyMoveCount_)
        );
        
        for (const auto& row : freqResult) {
            int assetId = row[0].as<int>();
            std::string epc = row[1].as<std::string>();
            int moveCount = row[2].as<int>();
            
            std::string message = "High frequency movement detected: " + std::to_string(moveCount) + 
                                 " moves in " + std::to_string(highFrequencyTimeWindowMinutes_) + " minutes";
            recordAnomaly(assetId, epc, AnomalyType::HIGH_FREQUENCY_MOVE, AnomalySeverity::WARNING, message);
        }
        
        if (!illegalLocations_.empty()) {
            std::stringstream locSS;
            locSS << "SELECT a.id, a.rfid_epc, a.location FROM assets a WHERE ";
            for (size_t i = 0; i < illegalLocations_.size(); ++i) {
                if (i > 0) locSS << " OR ";
                locSS << "a.location = " << W.quote(illegalLocations_[i]);
            }
            
            pqxx::result illegalResult = W.exec(locSS.str());
            
            for (const auto& row : illegalResult) {
                int assetId = row[0].as<int>();
                std::string epc = row[1].as<std::string>();
                std::string location = row[2].as<std::string>();
                
                std::string message = "Asset detected in restricted area: " + location;
                recordAnomaly(assetId, epc, AnomalyType::ILLEGAL_LOCATION, AnomalySeverity::ERROR, message, location);
            }
        }
        
        DBPool::instance().release(conn);
        
    } catch (const std::exception& e) {
        spdlog::error("Failed to scan for anomalies: {}", e.what());
    }
}

void AnomalyDetector::detectAssetMissing(int assetId, const std::string& epc) {
    try {
        auto conn = DBPool::instance().acquire();
        pqxx::work W(*conn);
        
        pqxx::result R = W.exec(
            "SELECT last_scan_time FROM scan_records WHERE asset_id = " + std::to_string(assetId) + " "
            "ORDER BY last_scan_time DESC LIMIT 1"
        );
        
        if (R.empty()) {
            std::string message = "Asset never scanned";
            recordAnomaly(assetId, epc, AnomalyType::ASSET_MISSING, AnomalySeverity::WARNING, message);
        } else {
            pqxx::result ageResult = W.exec(
                "SELECT EXTRACT(MINUTE FROM NOW() - last_scan_time) FROM scan_records "
                "WHERE asset_id = " + std::to_string(assetId) + " ORDER BY last_scan_time DESC LIMIT 1"
            );
            
            int minutesSinceLastScan = ageResult[0][0].as<int>();
            if (minutesSinceLastScan > missingThresholdMinutes_) {
                std::string message = "Asset not scanned for " + std::to_string(minutesSinceLastScan) + " minutes";
                recordAnomaly(assetId, epc, AnomalyType::ASSET_MISSING, AnomalySeverity::WARNING, message);
            }
        }
        
        DBPool::instance().release(conn);
        
    } catch (const std::exception& e) {
        spdlog::error("Failed to detect missing asset: {}", e.what());
    }
}

void AnomalyDetector::detectHighFrequencyMove(int assetId, const std::string& epc) {
    try {
        auto conn = DBPool::instance().acquire();
        pqxx::work W(*conn);
        
        pqxx::result R = W.exec(
            "SELECT COUNT(*) FROM asset_location_history "
            "WHERE asset_id = " + std::to_string(assetId) + " "
            "AND entry_time >= NOW() - INTERVAL '" + std::to_string(highFrequencyTimeWindowMinutes_) + " minutes'"
        );
        
        int moveCount = R[0][0].as<int>();
        if (moveCount > highFrequencyMoveCount_) {
            std::string message = "High frequency movement: " + std::to_string(moveCount) + 
                                 " moves in " + std::to_string(highFrequencyTimeWindowMinutes_) + " minutes";
            recordAnomaly(assetId, epc, AnomalyType::HIGH_FREQUENCY_MOVE, AnomalySeverity::WARNING, message);
        }
        
        DBPool::instance().release(conn);
        
    } catch (const std::exception& e) {
        spdlog::error("Failed to detect high frequency move: {}", e.what());
    }
}

void AnomalyDetector::detectIllegalLocation(int assetId, const std::string& epc, const std::string& location) {
    if (std::find(illegalLocations_.begin(), illegalLocations_.end(), location) != illegalLocations_.end()) {
        std::string message = "Asset detected in restricted area: " + location;
        recordAnomaly(assetId, epc, AnomalyType::ILLEGAL_LOCATION, AnomalySeverity::ERROR, message, location);
    }
}

std::vector<AnomalyEvent> AnomalyDetector::getActiveAnomalies() {
    std::vector<AnomalyEvent> events;
    
    try {
        auto conn = DBPool::instance().acquire();
        pqxx::work W(*conn);
        
        pqxx::result R = W.exec(
            "SELECT id, asset_id, epc, anomaly_type, severity, message, location, "
            "EXTRACT(EPOCH FROM detected_at)::BIGINT as det_ts, "
            "EXTRACT(EPOCH FROM resolved_at)::BIGINT as res_ts, "
            "resolved_by, is_resolved "
            "FROM anomaly_events WHERE is_resolved = FALSE ORDER BY detected_at DESC"
        );
        
        for (const auto& row : R) {
            AnomalyEvent event;
            event.id = row[0].as<int>();
            event.asset_id = row[1].as<int>();
            event.epc = row[2].as<std::string>();
            
            std::string typeStr = row[3].as<std::string>();
            if (typeStr == "ASSET_MISSING") event.type = AnomalyType::ASSET_MISSING;
            else if (typeStr == "HIGH_FREQUENCY_MOVE") event.type = AnomalyType::HIGH_FREQUENCY_MOVE;
            else if (typeStr == "ILLEGAL_LOCATION") event.type = AnomalyType::ILLEGAL_LOCATION;
            else if (typeStr == "STALE_DATA") event.type = AnomalyType::STALE_DATA;
            else if (typeStr == "UNEXPECTED_READER") event.type = AnomalyType::UNEXPECTED_READER;
            else if (typeStr == "DUPLICATE_EPC") event.type = AnomalyType::DUPLICATE_EPC;
            
            std::string sevStr = row[4].as<std::string>();
            if (sevStr == "INFO") event.severity = AnomalySeverity::INFO;
            else if (sevStr == "WARNING") event.severity = AnomalySeverity::WARNING;
            else if (sevStr == "ERROR") event.severity = AnomalySeverity::ERROR;
            else if (sevStr == "CRITICAL") event.severity = AnomalySeverity::CRITICAL;
            
            event.message = row[5].as<std::string>();
            event.location = row[6].as<std::string>();
            event.detected_at = row[7].is_null() ? 0 : row[7].as<int64_t>();
            event.resolved_at = row[8].is_null() ? 0 : row[8].as<int64_t>();
            event.resolved_by = row[9].as<std::string>();
            event.is_resolved = row[10].as<bool>();
            
            events.push_back(event);
        }
        
        DBPool::instance().release(conn);
        
    } catch (const std::exception& e) {
        spdlog::error("Failed to get active anomalies: {}", e.what());
    }
    
    return events;
}

std::vector<AnomalyEvent> AnomalyDetector::getAnomaliesByAsset(int assetId) {
    std::vector<AnomalyEvent> events;
    
    try {
        auto conn = DBPool::instance().acquire();
        pqxx::work W(*conn);
        
        pqxx::result R = W.exec(
            "SELECT id, asset_id, epc, anomaly_type, severity, message, location, "
            "EXTRACT(EPOCH FROM detected_at)::BIGINT as det_ts, "
            "EXTRACT(EPOCH FROM resolved_at)::BIGINT as res_ts, "
            "resolved_by, is_resolved "
            "FROM anomaly_events WHERE asset_id = " + std::to_string(assetId) + " ORDER BY detected_at DESC"
        );
        
        for (const auto& row : R) {
            AnomalyEvent event;
            event.id = row[0].as<int>();
            event.asset_id = row[1].as<int>();
            event.epc = row[2].as<std::string>();
            
            std::string typeStr = row[3].as<std::string>();
            if (typeStr == "ASSET_MISSING") event.type = AnomalyType::ASSET_MISSING;
            else if (typeStr == "HIGH_FREQUENCY_MOVE") event.type = AnomalyType::HIGH_FREQUENCY_MOVE;
            else if (typeStr == "ILLEGAL_LOCATION") event.type = AnomalyType::ILLEGAL_LOCATION;
            
            std::string sevStr = row[4].as<std::string>();
            if (sevStr == "INFO") event.severity = AnomalySeverity::INFO;
            else if (sevStr == "WARNING") event.severity = AnomalySeverity::WARNING;
            else if (sevStr == "ERROR") event.severity = AnomalySeverity::ERROR;
            else if (sevStr == "CRITICAL") event.severity = AnomalySeverity::CRITICAL;
            
            event.message = row[5].as<std::string>();
            event.location = row[6].as<std::string>();
            event.detected_at = row[7].is_null() ? 0 : row[7].as<int64_t>();
            event.resolved_at = row[8].is_null() ? 0 : row[8].as<int64_t>();
            event.resolved_by = row[9].as<std::string>();
            event.is_resolved = row[10].as<bool>();
            
            events.push_back(event);
        }
        
        DBPool::instance().release(conn);
        
    } catch (const std::exception& e) {
        spdlog::error("Failed to get anomalies by asset: {}", e.what());
    }
    
    return events;
}

std::vector<AnomalyEvent> AnomalyDetector::getAnomaliesByType(AnomalyType type) {
    std::vector<AnomalyEvent> events;
    
    try {
        auto conn = DBPool::instance().acquire();
        pqxx::work W(*conn);
        
        pqxx::result R = W.exec(
            "SELECT id, asset_id, epc, anomaly_type, severity, message, location, "
            "EXTRACT(EPOCH FROM detected_at)::BIGINT as det_ts, "
            "EXTRACT(EPOCH FROM resolved_at)::BIGINT as res_ts, "
            "resolved_by, is_resolved "
            "FROM anomaly_events WHERE anomaly_type = " + W.quote(typeToString(type)) + " ORDER BY detected_at DESC"
        );
        
        for (const auto& row : R) {
            AnomalyEvent event;
            event.id = row[0].as<int>();
            event.asset_id = row[1].as<int>();
            event.epc = row[2].as<std::string>();
            event.type = type;
            
            std::string sevStr = row[4].as<std::string>();
            if (sevStr == "INFO") event.severity = AnomalySeverity::INFO;
            else if (sevStr == "WARNING") event.severity = AnomalySeverity::WARNING;
            else if (sevStr == "ERROR") event.severity = AnomalySeverity::ERROR;
            else if (sevStr == "CRITICAL") event.severity = AnomalySeverity::CRITICAL;
            
            event.message = row[5].as<std::string>();
            event.location = row[6].as<std::string>();
            event.detected_at = row[7].is_null() ? 0 : row[7].as<int64_t>();
            event.resolved_at = row[8].is_null() ? 0 : row[8].as<int64_t>();
            event.resolved_by = row[9].as<std::string>();
            event.is_resolved = row[10].as<bool>();
            
            events.push_back(event);
        }
        
        DBPool::instance().release(conn);
        
    } catch (const std::exception& e) {
        spdlog::error("Failed to get anomalies by type: {}", e.what());
    }
    
    return events;
}

bool AnomalyDetector::resolveAnomaly(int anomalyId, const std::string& resolver) {
    try {
        auto conn = DBPool::instance().acquire();
        pqxx::work W(*conn);
        
        W.exec(
            "UPDATE anomaly_events SET is_resolved = TRUE, resolved_at = NOW(), resolved_by = " + W.quote(resolver) + 
            " WHERE id = " + std::to_string(anomalyId)
        );
        
        W.commit();
        DBPool::instance().release(conn);
        
        spdlog::info("Anomaly resolved: id={}, by={}", anomalyId, resolver);
        return true;
        
    } catch (const std::exception& e) {
        spdlog::error("Failed to resolve anomaly: {}", e.what());
        return false;
    }
}

int AnomalyDetector::getActiveAnomalyCount() {
    try {
        auto conn = DBPool::instance().acquire();
        pqxx::work W(*conn);
        
        pqxx::result R = W.exec("SELECT COUNT(*) FROM anomaly_events WHERE is_resolved = FALSE");
        
        DBPool::instance().release(conn);
        
        return R[0][0].as<int>();
        
    } catch (const std::exception& e) {
        spdlog::error("Failed to get active anomaly count: {}", e.what());
        return 0;
    }
}

} // namespace analytics