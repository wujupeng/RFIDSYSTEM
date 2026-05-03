#include "trajectory_engine.h"
#include "../db/db_pool.h"
#include "../core/logger.h"
#include <sstream>

namespace analytics {

TrajectoryEngine& TrajectoryEngine::instance() {
    static TrajectoryEngine instance;
    return instance;
}

TrajectoryEngine::TrajectoryEngine() {
    spdlog::info("TrajectoryEngine initialized");
}

std::string TrajectoryEngine::getPreviousLocation(int assetId) {
    try {
        auto conn = DBPool::instance().acquire();
        pqxx::work W(*conn);
        
        pqxx::result R = W.exec(
            "SELECT location FROM asset_location_history "
            "WHERE asset_id = " + W.to_string(assetId) + " "
            "AND exit_time IS NULL "
            "ORDER BY entry_time DESC LIMIT 1"
        );
        
        DBPool::instance().release(conn);
        
        if (!R.empty()) {
            return R[0][0].as<std::string>();
        }
    } catch (const std::exception& e) {
        spdlog::error("Failed to get previous location: {}", e.what());
    }
    return "";
}

void TrajectoryEngine::recordLocation(int assetId, const std::string& epc, const std::string& location, 
                                      const std::string& sourceReader) {
    try {
        auto conn = DBPool::instance().acquire();
        pqxx::work W(*conn);
        
        std::string previousLocation = getPreviousLocation(assetId);
        
        if (!previousLocation.empty() && previousLocation == location) {
            W.exec(
                "UPDATE asset_location_history SET "
                "duration_seconds = EXTRACT(EPOCH FROM NOW()) - EXTRACT(EPOCH FROM entry_time)::INT "
                "WHERE asset_id = " + W.to_string(assetId) + " "
                "AND exit_time IS NULL"
            );
            W.commit();
            DBPool::instance().release(conn);
            return;
        }
        
        if (!previousLocation.empty()) {
            W.exec(
                "UPDATE asset_location_history SET "
                "exit_time = NOW(), "
                "duration_seconds = EXTRACT(EPOCH FROM NOW()) - EXTRACT(EPOCH FROM entry_time)::INT "
                "WHERE asset_id = " + W.to_string(assetId) + " "
                "AND exit_time IS NULL"
            );
        }
        
        W.exec(
            "INSERT INTO asset_location_history "
            "(asset_id, epc, location, previous_location, source_reader, entry_time) "
            "VALUES(" +
            W.to_string(assetId) + ", " +
            W.quote(epc) + ", " +
            W.quote(location) + ", " +
            W.quote(previousLocation) + ", " +
            W.quote(sourceReader) + ", NOW())"
        );
        
        W.exec(
            "UPDATE assets SET location = " + W.quote(location) + " WHERE id = " + W.to_string(assetId)
        );
        
        W.commit();
        DBPool::instance().release(conn);
        
        spdlog::debug("Location recorded: asset={}, epc={}, location={}, previous={}", 
                      assetId, epc, location, previousLocation);
                      
    } catch (const std::exception& e) {
        spdlog::error("Failed to record location: {}", e.what());
    }
}

Trajectory TrajectoryEngine::getTrajectory(int assetId, int64_t startTime, int64_t endTime) {
    Trajectory trajectory;
    trajectory.asset_id = assetId;
    
    try {
        auto conn = DBPool::instance().acquire();
        pqxx::work W(*conn);
        
        std::stringstream ss;
        ss << "SELECT epc, location, EXTRACT(EPOCH FROM entry_time)::BIGINT as ts, source_reader "
           << "FROM asset_location_history WHERE asset_id = " << W.to_string(assetId);
        
        if (startTime > 0) {
            ss << " AND entry_time >= TO_TIMESTAMP(" << startTime << ")";
        }
        if (endTime > 0) {
            ss << " AND entry_time <= TO_TIMESTAMP(" << endTime << ")";
        }
        
        ss << " ORDER BY entry_time";
        
        pqxx::result R = W.exec(ss.str());
        
        for (const auto& row : R) {
            TrajectoryPoint point;
            point.location = row["location"].as<std::string>();
            point.timestamp = row["ts"].as<int64_t>();
            point.reader_id = row["source_reader"].as<std::string>();
            trajectory.points.push_back(point);
            
            if (trajectory.epc.empty()) {
                trajectory.epc = row["epc"].as<std::string>();
            }
        }
        
        DBPool::instance().release(conn);
        
    } catch (const std::exception& e) {
        spdlog::error("Failed to get trajectory: {}", e.what());
    }
    
    return trajectory;
}

Trajectory TrajectoryEngine::getTrajectoryByEPC(const std::string& epc, int64_t startTime, int64_t endTime) {
    Trajectory trajectory;
    trajectory.epc = epc;
    
    try {
        auto conn = DBPool::instance().acquire();
        pqxx::work W(*conn);
        
        std::stringstream ss;
        ss << "SELECT asset_id, location, EXTRACT(EPOCH FROM entry_time)::BIGINT as ts, source_reader "
           << "FROM asset_location_history WHERE epc = " << W.quote(epc);
        
        if (startTime > 0) {
            ss << " AND entry_time >= TO_TIMESTAMP(" << startTime << ")";
        }
        if (endTime > 0) {
            ss << " AND entry_time <= TO_TIMESTAMP(" << endTime << ")";
        }
        
        ss << " ORDER BY entry_time";
        
        pqxx::result R = W.exec(ss.str());
        
        for (const auto& row : R) {
            TrajectoryPoint point;
            point.location = row["location"].as<std::string>();
            point.timestamp = row["ts"].as<int64_t>();
            point.reader_id = row["source_reader"].as<std::string>();
            trajectory.points.push_back(point);
            
            if (trajectory.asset_id == 0) {
                trajectory.asset_id = row["asset_id"].as<int>();
            }
        }
        
        DBPool::instance().release(conn);
        
    } catch (const std::exception& e) {
        spdlog::error("Failed to get trajectory by EPC: {}", e.what());
    }
    
    return trajectory;
}

LocationRecord TrajectoryEngine::getCurrentLocation(int assetId) {
    LocationRecord record{};
    
    try {
        auto conn = DBPool::instance().acquire();
        pqxx::work W(*conn);
        
        pqxx::result R = W.exec(
            "SELECT id, epc, location, previous_location, "
            "EXTRACT(EPOCH FROM entry_time)::BIGINT as entry_ts, "
            "EXTRACT(EPOCH FROM exit_time)::BIGINT as exit_ts, "
            "duration_seconds, source_reader, confidence, event_type "
            "FROM asset_location_history "
            "WHERE asset_id = " + W.to_string(assetId) + " "
            "AND exit_time IS NULL "
            "ORDER BY entry_time DESC LIMIT 1"
        );
        
        if (!R.empty()) {
            record.id = R[0][0].as<int>();
            record.asset_id = assetId;
            record.epc = R[0][1].as<std::string>();
            record.location = R[0][2].as<std::string>();
            record.previous_location = R[0][3].as<std::string>();
            record.entry_time = R[0][4].as<int64_t>();
            record.exit_time = R[0][5].is_null() ? 0 : R[0][5].as<int64_t>();
            record.duration_seconds = R[0][6].as<int>();
            record.source_reader = R[0][7].as<std::string>();
            record.confidence = R[0][8].as<double>();
            record.event_type = R[0][9].as<std::string>();
        }
        
        DBPool::instance().release(conn);
        
    } catch (const std::exception& e) {
        spdlog::error("Failed to get current location: {}", e.what());
    }
    
    return record;
}

LocationRecord TrajectoryEngine::getCurrentLocationByEPC(const std::string& epc) {
    LocationRecord record{};
    
    try {
        auto conn = DBPool::instance().acquire();
        pqxx::work W(*conn);
        
        pqxx::result R = W.exec(
            "SELECT id, asset_id, location, previous_location, "
            "EXTRACT(EPOCH FROM entry_time)::BIGINT as entry_ts, "
            "EXTRACT(EPOCH FROM exit_time)::BIGINT as exit_ts, "
            "duration_seconds, source_reader, confidence, event_type "
            "FROM asset_location_history "
            "WHERE epc = " + W.quote(epc) + " "
            "AND exit_time IS NULL "
            "ORDER BY entry_time DESC LIMIT 1"
        );
        
        if (!R.empty()) {
            record.id = R[0][0].as<int>();
            record.asset_id = R[0][1].as<int>();
            record.epc = epc;
            record.location = R[0][2].as<std::string>();
            record.previous_location = R[0][3].as<std::string>();
            record.entry_time = R[0][4].as<int64_t>();
            record.exit_time = R[0][5].is_null() ? 0 : R[0][5].as<int64_t>();
            record.duration_seconds = R[0][6].as<int>();
            record.source_reader = R[0][7].as<std::string>();
            record.confidence = R[0][8].as<double>();
            record.event_type = R[0][9].as<std::string>();
        }
        
        DBPool::instance().release(conn);
        
    } catch (const std::exception& e) {
        spdlog::error("Failed to get current location by EPC: {}", e.what());
    }
    
    return record;
}

std::vector<LocationRecord> TrajectoryEngine::getLocationHistory(int assetId, int limit) {
    std::vector<LocationRecord> records;
    
    try {
        auto conn = DBPool::instance().acquire();
        pqxx::work W(*conn);
        
        pqxx::result R = W.exec(
            "SELECT id, epc, location, previous_location, "
            "EXTRACT(EPOCH FROM entry_time)::BIGINT as entry_ts, "
            "EXTRACT(EPOCH FROM exit_time)::BIGINT as exit_ts, "
            "duration_seconds, source_reader, confidence, event_type "
            "FROM asset_location_history "
            "WHERE asset_id = " + W.to_string(assetId) + " "
            "ORDER BY entry_time DESC LIMIT " + W.to_string(limit)
        );
        
        for (const auto& row : R) {
            LocationRecord record;
            record.id = row[0].as<int>();
            record.asset_id = assetId;
            record.epc = row[1].as<std::string>();
            record.location = row[2].as<std::string>();
            record.previous_location = row[3].as<std::string>();
            record.entry_time = row[4].is_null() ? 0 : row[4].as<int64_t>();
            record.exit_time = row[5].is_null() ? 0 : row[5].as<int64_t>();
            record.duration_seconds = row[6].as<int>();
            record.source_reader = row[7].as<std::string>();
            record.confidence = row[8].as<double>();
            record.event_type = row[9].as<std::string>();
            records.push_back(record);
        }
        
        DBPool::instance().release(conn);
        
    } catch (const std::exception& e) {
        spdlog::error("Failed to get location history: {}", e.what());
    }
    
    return records;
}

std::vector<LocationRecord> TrajectoryEngine::getLocationHistoryByEPC(const std::string& epc, int limit) {
    std::vector<LocationRecord> records;
    
    try {
        auto conn = DBPool::instance().acquire();
        pqxx::work W(*conn);
        
        pqxx::result R = W.exec(
            "SELECT id, asset_id, location, previous_location, "
            "EXTRACT(EPOCH FROM entry_time)::BIGINT as entry_ts, "
            "EXTRACT(EPOCH FROM exit_time)::BIGINT as exit_ts, "
            "duration_seconds, source_reader, confidence, event_type "
            "FROM asset_location_history "
            "WHERE epc = " + W.quote(epc) + " "
            "ORDER BY entry_time DESC LIMIT " + W.to_string(limit)
        );
        
        for (const auto& row : R) {
            LocationRecord record;
            record.id = row[0].as<int>();
            record.asset_id = row[1].as<int>();
            record.epc = epc;
            record.location = row[2].as<std::string>();
            record.previous_location = row[3].as<std::string>();
            record.entry_time = row[4].is_null() ? 0 : row[4].as<int64_t>();
            record.exit_time = row[5].is_null() ? 0 : row[5].as<int64_t>();
            record.duration_seconds = row[6].as<int>();
            record.source_reader = row[7].as<std::string>();
            record.confidence = row[8].as<double>();
            record.event_type = row[9].as<std::string>();
            records.push_back(record);
        }
        
        DBPool::instance().release(conn);
        
    } catch (const std::exception& e) {
        spdlog::error("Failed to get location history by EPC: {}", e.what());
    }
    
    return records;
}

int TrajectoryEngine::getMoveCount(int assetId, int64_t startTime, int64_t endTime) {
    try {
        auto conn = DBPool::instance().acquire();
        pqxx::work W(*conn);
        
        std::stringstream ss;
        ss << "SELECT COUNT(*) FROM asset_location_history WHERE asset_id = " << W.to_string(assetId);
        
        if (startTime > 0) {
            ss << " AND entry_time >= TO_TIMESTAMP(" << startTime << ")";
        }
        if (endTime > 0) {
            ss << " AND entry_time <= TO_TIMESTAMP(" << endTime << ")";
        }
        
        pqxx::result R = W.exec(ss.str());
        
        DBPool::instance().release(conn);
        
        return R[0][0].as<int>();
        
    } catch (const std::exception& e) {
        spdlog::error("Failed to get move count: {}", e.what());
        return 0;
    }
}

void TrajectoryEngine::closeLocation(int assetId, const std::string& newLocation) {
    try {
        auto conn = DBPool::instance().acquire();
        pqxx::work W(*conn);
        
        W.exec(
            "UPDATE asset_location_history SET "
            "exit_time = NOW(), "
            "duration_seconds = EXTRACT(EPOCH FROM NOW()) - EXTRACT(EPOCH FROM entry_time)::INT "
            "WHERE asset_id = " + W.to_string(assetId) + " "
            "AND exit_time IS NULL"
        );
        
        if (!newLocation.empty()) {
            W.exec(
                "UPDATE assets SET location = " + W.quote(newLocation) + " WHERE id = " + W.to_string(assetId)
            );
        }
        
        W.commit();
        DBPool::instance().release(conn);
        
    } catch (const std::exception& e) {
        spdlog::error("Failed to close location: {}", e.what());
    }
}

} // namespace analytics