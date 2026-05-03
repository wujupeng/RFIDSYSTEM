#pragma once
#include <string>
#include <vector>
#include <memory>

namespace analytics {

struct LocationRecord {
    int id;
    int asset_id;
    std::string epc;
    std::string location;
    std::string previous_location;
    int64_t entry_time;
    int64_t exit_time;
    int duration_seconds;
    std::string source_reader;
    double confidence;
    std::string event_type;
};

struct TrajectoryPoint {
    std::string location;
    int64_t timestamp;
    std::string reader_id;
};

struct Trajectory {
    int asset_id;
    std::string epc;
    std::vector<TrajectoryPoint> points;
};

struct BehaviorStats {
    int asset_id;
    std::string epc;
    int unique_locations;
    int total_moves;
    double avg_stay_duration;
    int max_stay_duration;
    int min_stay_duration;
    std::string current_location;
    std::string first_location;
};

class TrajectoryEngine {
public:
    static TrajectoryEngine& instance();
    
    void recordLocation(int assetId, const std::string& epc, const std::string& location, 
                        const std::string& sourceReader = "");
    
    Trajectory getTrajectory(int assetId, int64_t startTime = 0, int64_t endTime = 0);
    
    Trajectory getTrajectoryByEPC(const std::string& epc, int64_t startTime = 0, int64_t endTime = 0);
    
    LocationRecord getCurrentLocation(int assetId);
    
    LocationRecord getCurrentLocationByEPC(const std::string& epc);
    
    std::vector<LocationRecord> getLocationHistory(int assetId, int limit = 100);
    
    std::vector<LocationRecord> getLocationHistoryByEPC(const std::string& epc, int limit = 100);
    
    int getMoveCount(int assetId, int64_t startTime = 0, int64_t endTime = 0);
    
    void closeLocation(int assetId, const std::string& newLocation = "");
    
private:
    TrajectoryEngine();
    ~TrajectoryEngine() = default;
    TrajectoryEngine(const TrajectoryEngine&) = delete;
    TrajectoryEngine& operator=(const TrajectoryEngine&) = delete;
    
    std::string getPreviousLocation(int assetId);
};

} // namespace analytics