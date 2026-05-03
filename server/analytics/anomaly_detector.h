#pragma once
#include <string>
#include <vector>
#include <memory>
#include <chrono>

namespace analytics {

enum class AnomalySeverity {
    INFO,
    WARNING,
    ERROR,
    CRITICAL
};

enum class AnomalyType {
    ASSET_MISSING,
    HIGH_FREQUENCY_MOVE,
    ILLEGAL_LOCATION,
    STALE_DATA,
    UNEXPECTED_READER,
    DUPLICATE_EPC
};

struct AnomalyEvent {
    int id;
    int asset_id;
    std::string epc;
    AnomalyType type;
    AnomalySeverity severity;
    std::string message;
    std::string location;
    int64_t detected_at;
    int64_t resolved_at;
    std::string resolved_by;
    bool is_resolved;
};

class AnomalyDetector {
public:
    static AnomalyDetector& instance();
    
    void setMissingThreshold(int minutes);
    
    void setHighFrequencyThreshold(int moves, int minutes);
    
    void setValidLocations(const std::vector<std::string>& locations);
    
    void addIllegalLocation(const std::string& location);
    
    void scanForAnomalies();
    
    void detectAssetMissing(int assetId, const std::string& epc);
    
    void detectHighFrequencyMove(int assetId, const std::string& epc);
    
    void detectIllegalLocation(int assetId, const std::string& epc, const std::string& location);
    
    std::vector<AnomalyEvent> getActiveAnomalies();
    
    std::vector<AnomalyEvent> getAnomaliesByAsset(int assetId);
    
    std::vector<AnomalyEvent> getAnomaliesByType(AnomalyType type);
    
    bool resolveAnomaly(int anomalyId, const std::string& resolver);
    
    int getActiveAnomalyCount();
    
private:
    AnomalyDetector();
    ~AnomalyDetector() = default;
    AnomalyDetector(const AnomalyDetector&) = delete;
    AnomalyDetector& operator=(const AnomalyDetector&) = delete;
    
    void recordAnomaly(int assetId, const std::string& epc, AnomalyType type, 
                       AnomalySeverity severity, const std::string& message, 
                       const std::string& location = "");
    
    std::string typeToString(AnomalyType type);
    std::string severityToString(AnomalySeverity severity);
    
    int missingThresholdMinutes_ = 30;
    int highFrequencyMoveCount_ = 10;
    int highFrequencyTimeWindowMinutes_ = 60;
    
    std::vector<std::string> validLocations_;
    std::vector<std::string> illegalLocations_;
};

} // namespace analytics