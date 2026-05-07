#include "trajectory_archive.h"
#include <cmath>
#include <algorithm>
#include <cstring>

TrajectoryArchive::TrajectoryArchive() 
    : maxHotPoints_(5000), currentHotPoints_(0), useColdStorage_(false) {
}

TrajectoryArchive::~TrajectoryArchive() {
    flushHotToCold();
    clear();
}

void TrajectoryArchive::initialize(int maxHotPoints, const std::string& coldStoragePath) {
    maxHotPoints_ = maxHotPoints;
    
    if (!coldStoragePath.empty()) {
        coldStoragePath_ = coldStoragePath;
        coldFile_.open(coldStoragePath, std::ios::binary | std::ios::app);
        useColdStorage_ = coldFile_.is_open();
    }
}

void TrajectoryArchive::appendPoint(int assetId, double x, double y, 
                                   int64_t timestamp, const std::string& location, 
                                   double riskScore) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    TrajectoryPoint point;
    point.x = x;
    point.y = y;
    point.timestamp = timestamp;
    point.location = location;
    point.risk_score = riskScore;
    
    hotLayer_[assetId].push_back(point);
    currentHotPoints_++;
    
    // Trim if needed
    trimHotLayer();
}

void TrajectoryArchive::appendPoints(int assetId, const std::vector<TrajectoryPoint>& points) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto& trajectory = hotLayer_[assetId];
    trajectory.insert(trajectory.end(), points.begin(), points.end());
    currentHotPoints_ += points.size();
    
    // Trim if needed
    trimHotLayer();
}

void TrajectoryArchive::trimHotLayer() {
    while (currentHotPoints_ > maxHotPoints_) {
        // Find asset with most points
        int maxAssetId = -1;
        size_t maxPoints = 0;
        
        for (const auto& pair : hotLayer_) {
            if (pair.second.size() > maxPoints) {
                maxPoints = pair.second.size();
                maxAssetId = pair.first;
            }
        }
        
        if (maxAssetId == -1) {
            break;
        }
        
        // Move oldest point to cold storage
        auto& trajectory = hotLayer_[maxAssetId];
        if (!trajectory.empty()) {
            const TrajectoryPoint& point = trajectory.front();
            
            if (useColdStorage_) {
                TrajectoryRecord record;
                record.asset_id = maxAssetId;
                record.x = point.x;
                record.y = point.y;
                record.timestamp = point.timestamp;
                record.risk_score = static_cast<float>(point.risk_score);
                record.location_length = static_cast<uint16_t>(point.location.size());
                
                std::streampos pos = coldFile_.tellp();
                writeColdRecord(record);
                coldIndex_[maxAssetId].push_back(pos);
            }
            
            trajectory.pop_front();
            currentHotPoints_--;
        }
    }
}

void TrajectoryArchive::flushHotToCold() {
    if (!useColdStorage_) {
        return;
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    for (const auto& pair : hotLayer_) {
        int assetId = pair.first;
        const auto& trajectory = pair.second;
        
        for (const auto& point : trajectory) {
            TrajectoryRecord record;
            record.asset_id = assetId;
            record.x = point.x;
            record.y = point.y;
            record.timestamp = point.timestamp;
            record.risk_score = static_cast<float>(point.risk_score);
            record.location_length = static_cast<uint16_t>(point.location.size());
            
            std::streampos pos = coldFile_.tellp();
            writeColdRecord(record);
            coldIndex_[assetId].push_back(pos);
        }
    }
    
    hotLayer_.clear();
    currentHotPoints_ = 0;
    
    coldFile_.flush();
}

void TrajectoryArchive::writeColdRecord(const TrajectoryRecord& record) {
    coldFile_.write(reinterpret_cast<const char*>(&record.asset_id), sizeof(record.asset_id));
    coldFile_.write(reinterpret_cast<const char*>(&record.x), sizeof(record.x));
    coldFile_.write(reinterpret_cast<const char*>(&record.y), sizeof(record.y));
    coldFile_.write(reinterpret_cast<const char*>(&record.timestamp), sizeof(record.timestamp));
    coldFile_.write(reinterpret_cast<const char*>(&record.risk_score), sizeof(record.risk_score));
    coldFile_.write(reinterpret_cast<const char*>(&record.location_length), sizeof(record.location_length));
}

bool TrajectoryArchive::readColdRecord(std::ifstream& file, TrajectoryRecord& record) {
    file.read(reinterpret_cast<char*>(&record.asset_id), sizeof(record.asset_id));
    file.read(reinterpret_cast<char*>(&record.x), sizeof(record.x));
    file.read(reinterpret_cast<char*>(&record.y), sizeof(record.y));
    file.read(reinterpret_cast<char*>(&record.timestamp), sizeof(record.timestamp));
    file.read(reinterpret_cast<char*>(&record.risk_score), sizeof(record.risk_score));
    file.read(reinterpret_cast<char*>(&record.location_length), sizeof(record.location_length));
    
    return file.good();
}

std::vector<TrajectoryPoint> TrajectoryArchive::getTrajectory(int assetId) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::vector<TrajectoryPoint> result;
    
    // Load from cold storage first
    std::vector<TrajectoryPoint> coldPoints = loadFromCold(assetId);
    result.insert(result.end(), coldPoints.begin(), coldPoints.end());
    
    // Add hot layer
    auto it = hotLayer_.find(assetId);
    if (it != hotLayer_.end()) {
        const auto& hotPoints = it->second;
        result.insert(result.end(), hotPoints.begin(), hotPoints.end());
    }
    
    return result;
}

std::vector<TrajectoryPoint> TrajectoryArchive::getRecentPoints(int assetId, int count) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = hotLayer_.find(assetId);
    if (it == hotLayer_.end()) {
        return {};
    }
    
    const auto& points = it->second;
    int startIdx = std::max(0, static_cast<int>(points.size()) - count);
    
    return std::vector<TrajectoryPoint>(points.begin() + startIdx, points.end());
}

std::vector<TrajectoryPoint> TrajectoryArchive::getTrajectoryRange(int assetId, 
                                                                  int64_t startTime, 
                                                                  int64_t endTime) const {
    std::vector<TrajectoryPoint> allPoints = getTrajectory(assetId);
    
    std::vector<TrajectoryPoint> result;
    for (const auto& point : allPoints) {
        if (point.timestamp >= startTime && point.timestamp <= endTime) {
            result.push_back(point);
        }
    }
    
    return result;
}

std::vector<TrajectoryPoint> TrajectoryArchive::loadFromCold(int assetId) const {
    if (!useColdStorage_) {
        return {};
    }
    
    auto it = coldIndex_.find(assetId);
    if (it == coldIndex_.end()) {
        return {};
    }
    
    std::ifstream file(coldStoragePath_, std::ios::binary);
    if (!file.is_open()) {
        return {};
    }
    
    std::vector<TrajectoryPoint> result;
    
    for (std::streampos pos : it->second) {
        file.seekg(pos);
        
        TrajectoryRecord record;
        if (!readColdRecord(file, record)) {
            continue;
        }
        
        TrajectoryPoint point;
        point.x = record.x;
        point.y = record.y;
        point.timestamp = record.timestamp;
        point.risk_score = record.risk_score;
        
        // Read location string
        if (record.location_length > 0) {
            std::vector<char> buffer(record.location_length);
            file.read(buffer.data(), record.location_length);
            point.location = std::string(buffer.data(), record.location_length);
        }
        
        result.push_back(point);
    }
    
    return result;
}

double TrajectoryArchive::getTotalDistance(int assetId) const {
    std::vector<TrajectoryPoint> points = getTrajectory(assetId);
    
    if (points.size() < 2) {
        return 0.0;
    }
    
    double totalDistance = 0.0;
    for (size_t i = 1; i < points.size(); ++i) {
        double dx = points[i].x - points[i-1].x;
        double dy = points[i].y - points[i-1].y;
        totalDistance += std::sqrt(dx * dx + dy * dy);
    }
    
    return totalDistance;
}

double TrajectoryArchive::getAverageSpeed(int assetId) const {
    std::vector<TrajectoryPoint> points = getTrajectory(assetId);
    
    if (points.size() < 2) {
        return 0.0;
    }
    
    double totalDistance = 0.0;
    for (size_t i = 1; i < points.size(); ++i) {
        double dx = points[i].x - points[i-1].x;
        double dy = points[i].y - points[i-1].y;
        totalDistance += std::sqrt(dx * dx + dy * dy);
    }
    
    double duration = (points.back().timestamp - points.front().timestamp) / 1000.0;
    
    if (duration <= 0) {
        return 0.0;
    }
    
    return totalDistance / duration;
}

std::string TrajectoryArchive::getMostFrequentLocation(int assetId) const {
    std::vector<TrajectoryPoint> points = getTrajectory(assetId);
    
    std::unordered_map<std::string, int> locationCounts;
    int maxCount = 0;
    std::string mostFrequent;
    
    for (const auto& point : points) {
        if (!point.location.empty()) {
            int count = ++locationCounts[point.location];
            if (count > maxCount) {
                maxCount = count;
                mostFrequent = point.location;
            }
        }
    }
    
    return mostFrequent;
}

void TrajectoryArchive::removeAsset(int assetId) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = hotLayer_.find(assetId);
    if (it != hotLayer_.end()) {
        currentHotPoints_ -= it->second.size();
        hotLayer_.erase(it);
    }
    
    coldIndex_.erase(assetId);
}

void TrajectoryArchive::clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    hotLayer_.clear();
    coldIndex_.clear();
    currentHotPoints_ = 0;
}

int TrajectoryArchive::assetCount() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return hotLayer_.size();
}

int TrajectoryArchive::totalPointCount() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return currentHotPoints_;
}

void TrajectoryArchive::setMaxHotPoints(int max) {
    std::lock_guard<std::mutex> lock(mutex_);
    maxHotPoints_ = max;
    trimHotLayer();
}

int TrajectoryArchive::maxHotPoints() const {
    return maxHotPoints_;
}