#include "behavior_analyzer.h"
#include "../db/db_pool.h"
#include "../core/logger.h"
#include <sstream>
#include <algorithm>

namespace analytics {

BehaviorAnalyzer& BehaviorAnalyzer::instance() {
    static BehaviorAnalyzer instance;
    return instance;
}

BehaviorAnalyzer::BehaviorAnalyzer() {
    spdlog::info("BehaviorAnalyzer initialized");
}

UsageStats BehaviorAnalyzer::analyzeUsage(int assetId) {
    UsageStats stats{};
    stats.asset_id = assetId;
    
    try {
        auto conn = DBPool::instance().acquire();
        pqxx::work W(*conn);
        
        pqxx::result scanResult = W.exec(
            "SELECT COUNT(*) FROM scan_records WHERE asset_id = " + std::to_string(assetId)
        );
        stats.total_scans = scanResult[0][0].as<int>();
        
        pqxx::result moveResult = W.exec(
            "SELECT COUNT(*) FROM asset_location_history WHERE asset_id = " + std::to_string(assetId)
        );
        stats.move_count = moveResult[0][0].as<int>();
        
        pqxx::result locResult = W.exec(
            "SELECT COUNT(DISTINCT location) FROM asset_location_history WHERE asset_id = " + std::to_string(assetId)
        );
        stats.unique_locations = locResult[0][0].as<int>();
        
        pqxx::result dailyResult = W.exec(
            "SELECT COUNT(*) / (EXTRACT(DAY FROM (NOW() - MIN(entry_time))) + 1) "
            "FROM asset_location_history WHERE asset_id = " + std::to_string(assetId)
        );
        stats.daily_avg_scans = dailyResult[0][0].as<double>();
        
        pqxx::result weeklyResult = W.exec(
            "SELECT COUNT(*) / (EXTRACT(WEEK FROM (NOW() - MIN(entry_time))) + 1) "
            "FROM asset_location_history WHERE asset_id = " + std::to_string(assetId)
        );
        stats.weekly_avg_scans = weeklyResult[0][0].as<double>();
        
        pqxx::result mostResult = W.exec(
            "SELECT location, COUNT(*) as cnt FROM asset_location_history "
            "WHERE asset_id = " + std::to_string(assetId) + " "
            "GROUP BY location ORDER BY cnt DESC LIMIT 1"
        );
        if (!mostResult.empty()) {
            stats.most_visited_location = mostResult[0][0].as<std::string>();
        }
        
        pqxx::result leastResult = W.exec(
            "SELECT location, COUNT(*) as cnt FROM asset_location_history "
            "WHERE asset_id = " + std::to_string(assetId) + " "
            "GROUP BY location ORDER BY cnt ASC LIMIT 1"
        );
        if (!leastResult.empty()) {
            stats.least_visited_location = leastResult[0][0].as<std::string>();
        }
        
        pqxx::result epcResult = W.exec(
            "SELECT epc FROM asset_location_history WHERE asset_id = " + std::to_string(assetId) + " LIMIT 1"
        );
        if (!epcResult.empty()) {
            stats.epc = epcResult[0][0].as<std::string>();
        }
        
        DBPool::instance().release(conn);
        
    } catch (const std::exception& e) {
        spdlog::error("Failed to analyze usage: {}", e.what());
    }
    
    return stats;
}

UsageStats BehaviorAnalyzer::analyzeUsageByEPC(const std::string& epc) {
    UsageStats stats{};
    stats.epc = epc;
    
    try {
        auto conn = DBPool::instance().acquire();
        pqxx::work W(*conn);
        
        pqxx::result assetResult = W.exec(
            "SELECT asset_id FROM assets WHERE rfid_epc = " + W.quote(epc)
        );
        if (!assetResult.empty()) {
            stats.asset_id = assetResult[0][0].as<int>();
        }
        
        pqxx::result scanResult = W.exec(
            "SELECT COUNT(*) FROM scan_records WHERE epc = " + W.quote(epc)
        );
        stats.total_scans = scanResult[0][0].as<int>();
        
        pqxx::result moveResult = W.exec(
            "SELECT COUNT(*) FROM asset_location_history WHERE epc = " + W.quote(epc)
        );
        stats.move_count = moveResult[0][0].as<int>();
        
        pqxx::result locResult = W.exec(
            "SELECT COUNT(DISTINCT location) FROM asset_location_history WHERE epc = " + W.quote(epc)
        );
        stats.unique_locations = locResult[0][0].as<int>();
        
        DBPool::instance().release(conn);
        
    } catch (const std::exception& e) {
        spdlog::error("Failed to analyze usage by EPC: {}", e.what());
    }
    
    return stats;
}

std::vector<BehaviorPattern> BehaviorAnalyzer::getBehaviorPatterns(int assetId) {
    std::vector<BehaviorPattern> patterns;
    
    try {
        auto conn = DBPool::instance().acquire();
        pqxx::work W(*conn);
        
        pqxx::result R = W.exec(
            "SELECT location, COUNT(*) as cnt, "
            "AVG(duration_seconds) as avg_dur, "
            "MAX(duration_seconds) as max_dur, "
            "MIN(duration_seconds) as min_dur "
            "FROM asset_location_history "
            "WHERE asset_id = " + std::to_string(assetId) + " "
            "GROUP BY location ORDER BY cnt DESC"
        );
        
        std::string epc;
        for (const auto& row : R) {
            BehaviorPattern pattern;
            pattern.asset_id = assetId;
            pattern.location = row["location"].as<std::string>();
            pattern.visit_count = row["cnt"].as<int>();
            pattern.avg_stay_duration = row["avg_dur"].as<double>();
            pattern.max_stay_duration = row["max_dur"].as<int>();
            pattern.min_stay_duration = row["min_dur"].as<int>();
            
            if (epc.empty()) {
                pqxx::result epcResult = W.exec(
                    "SELECT epc FROM asset_location_history WHERE asset_id = " + std::to_string(assetId) + " LIMIT 1"
                );
                if (!epcResult.empty()) {
                    epc = epcResult[0][0].as<std::string>();
                }
            }
            pattern.epc = epc;
            
            patterns.push_back(pattern);
        }
        
        DBPool::instance().release(conn);
        
    } catch (const std::exception& e) {
        spdlog::error("Failed to get behavior patterns: {}", e.what());
    }
    
    return patterns;
}

std::vector<BehaviorPattern> BehaviorAnalyzer::getBehaviorPatternsByEPC(const std::string& epc) {
    std::vector<BehaviorPattern> patterns;
    
    try {
        auto conn = DBPool::instance().acquire();
        pqxx::work W(*conn);
        
        pqxx::result R = W.exec(
            "SELECT asset_id, location, COUNT(*) as cnt, "
            "AVG(duration_seconds) as avg_dur, "
            "MAX(duration_seconds) as max_dur, "
            "MIN(duration_seconds) as min_dur "
            "FROM asset_location_history "
            "WHERE epc = " + W.quote(epc) + " "
            "GROUP BY asset_id, location ORDER BY cnt DESC"
        );
        
        for (const auto& row : R) {
            BehaviorPattern pattern;
            pattern.asset_id = row["asset_id"].as<int>();
            pattern.epc = epc;
            pattern.location = row["location"].as<std::string>();
            pattern.visit_count = row["cnt"].as<int>();
            pattern.avg_stay_duration = row["avg_dur"].as<double>();
            pattern.max_stay_duration = row["max_dur"].as<int>();
            pattern.min_stay_duration = row["min_dur"].as<int>();
            
            patterns.push_back(pattern);
        }
        
        DBPool::instance().release(conn);
        
    } catch (const std::exception& e) {
        spdlog::error("Failed to get behavior patterns by EPC: {}", e.what());
    }
    
    return patterns;
}

int BehaviorAnalyzer::getUsageFrequency(int assetId, int64_t startTime, int64_t endTime) {
    try {
        auto conn = DBPool::instance().acquire();
        pqxx::work W(*conn);
        
        std::stringstream ss;
        ss << "SELECT COUNT(*) FROM scan_records WHERE asset_id = " << std::to_string(assetId);
        
        if (startTime > 0) {
            ss << " AND first_scan_time >= TO_TIMESTAMP(" << startTime << ")";
        }
        if (endTime > 0) {
            ss << " AND last_scan_time <= TO_TIMESTAMP(" << endTime << ")";
        }
        
        pqxx::result R = W.exec(ss.str());
        
        DBPool::instance().release(conn);
        
        return R[0][0].as<int>();
        
    } catch (const std::exception& e) {
        spdlog::error("Failed to get usage frequency: {}", e.what());
        return 0;
    }
}

double BehaviorAnalyzer::getAverageStayDuration(int assetId) {
    try {
        auto conn = DBPool::instance().acquire();
        pqxx::work W(*conn);
        
        pqxx::result R = W.exec(
            "SELECT AVG(duration_seconds) FROM asset_location_history "
            "WHERE asset_id = " + std::to_string(assetId) + " AND duration_seconds > 0"
        );
        
        DBPool::instance().release(conn);
        
        return R[0][0].as<double>();
        
    } catch (const std::exception& e) {
        spdlog::error("Failed to get average stay duration: {}", e.what());
        return 0.0;
    }
}

std::string BehaviorAnalyzer::getMostVisitedLocation(int assetId) {
    try {
        auto conn = DBPool::instance().acquire();
        pqxx::work W(*conn);
        
        pqxx::result R = W.exec(
            "SELECT location FROM asset_location_history "
            "WHERE asset_id = " + std::to_string(assetId) + " "
            "GROUP BY location ORDER BY COUNT(*) DESC LIMIT 1"
        );
        
        DBPool::instance().release(conn);
        
        if (!R.empty()) {
            return R[0][0].as<std::string>();
        }
        
    } catch (const std::exception& e) {
        spdlog::error("Failed to get most visited location: {}", e.what());
    }
    
    return "";
}

std::vector<std::pair<std::string, int>> BehaviorAnalyzer::getLocationVisitCounts(int assetId) {
    std::vector<std::pair<std::string, int>> counts;
    
    try {
        auto conn = DBPool::instance().acquire();
        pqxx::work W(*conn);
        
        pqxx::result R = W.exec(
            "SELECT location, COUNT(*) as cnt FROM asset_location_history "
            "WHERE asset_id = " + std::to_string(assetId) + " "
            "GROUP BY location ORDER BY cnt DESC"
        );
        
        for (const auto& row : R) {
            counts.emplace_back(row["location"].as<std::string>(), row["cnt"].as<int>());
        }
        
        DBPool::instance().release(conn);
        
    } catch (const std::exception& e) {
        spdlog::error("Failed to get location visit counts: {}", e.what());
    }
    
    return counts;
}

} // namespace analytics