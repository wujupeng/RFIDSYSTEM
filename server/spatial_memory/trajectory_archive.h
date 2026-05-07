#pragma once

#include <unordered_map>
#include <deque>
#include <mutex>
#include <string>
#include <vector>
#include <fstream>
#include <memory>

struct TrajectoryPoint {
    double x;
    double y;
    int64_t timestamp;
    std::string location;
    double risk_score;
};

// Binary format for cold storage
struct TrajectoryRecord {
    int32_t asset_id;
    double x;
    double y;
    int64_t timestamp;
    float risk_score;
    uint16_t location_length;
    // Followed by location string
};

class TrajectoryArchive {
public:
    TrajectoryArchive();
    ~TrajectoryArchive();
    
    void initialize(int maxHotPoints = 5000, const std::string& coldStoragePath = "");
    
    // Append-only operations
    void appendPoint(int assetId, double x, double y, 
                     int64_t timestamp, const std::string& location = "", 
                     double riskScore = 0.0);
    
    void appendPoints(int assetId, const std::vector<TrajectoryPoint>& points);
    
    // Query operations
    std::vector<TrajectoryPoint> getTrajectory(int assetId) const;
    
    std::vector<TrajectoryPoint> getRecentPoints(int assetId, int count) const;
    
    std::vector<TrajectoryPoint> getTrajectoryRange(int assetId, 
                                                    int64_t startTime, 
                                                    int64_t endTime) const;
    
    // Statistics
    double getTotalDistance(int assetId) const;
    
    double getAverageSpeed(int assetId) const;
    
    std::string getMostFrequentLocation(int assetId) const;
    
    // Management
    void removeAsset(int assetId);
    
    void clear();
    
    int assetCount() const;
    
    int totalPointCount() const;
    
    // Configuration
    void setMaxHotPoints(int max);
    int maxHotPoints() const;
    
private:
    // Flush hot layer to cold storage
    void flushHotToCold();
    
    // Load from cold storage
    std::vector<TrajectoryPoint> loadFromCold(int assetId) const;
    
    // Write record to cold storage
    void writeColdRecord(const TrajectoryRecord& record);
    
    // Read record from cold storage
    bool readColdRecord(std::ifstream& file, TrajectoryRecord& record);
    
    // Trim excess hot points
    void trimHotLayer();
    
    mutable std::mutex mutex_;
    
    // Hot Layer: Recent data in memory (last ~5 minutes)
    std::unordered_map<int, std::deque<TrajectoryPoint>> hotLayer_;
    int maxHotPoints_;
    int currentHotPoints_;
    
    // Cold Layer: mmap file for long-term storage
    std::string coldStoragePath_;
    std::ofstream coldFile_;
    bool useColdStorage_;
    
    // Index for cold storage
    std::unordered_map<int, std::vector<std::streampos>> coldIndex_;
};