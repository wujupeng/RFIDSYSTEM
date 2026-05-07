#pragma once

#include <QObject>
#include <QMutex>
#include <QMap>
#include <QVector>
#include <memory>

struct ZoneStats {
    qint64 timestamp;
    int deviceCount;
    float avgStayTimeMinutes;
    float inflowRate;
    float outflowRate;
    double congestionScore;
    std::string status;
};

struct ZoneHistory {
    int zoneId;
    std::string zoneName;
    QVector<ZoneStats> history;
    int maxHistorySize;
    
    ZoneHistory() : maxHistorySize(600) {} // 10 minutes at 1Hz
};

class ZoneHistoryManager : public QObject {
    Q_OBJECT
    
public:
    ZoneHistoryManager(QObject* parent = nullptr);
    ~ZoneHistoryManager();
    
    void initialize();
    
    void updateZone(int zoneId, const std::string& zoneName,
                    int deviceCount, float avgStayTimeMinutes,
                    float inflowRate, float outflowRate,
                    double congestionScore, const std::string& status);
    
    QVector<ZoneStats> getZoneHistory(int zoneId, qint64 startTime, qint64 endTime) const;
    
    QVector<ZoneStats> getRecentStats(int zoneId, int count) const;
    
    void removeZone(int zoneId);
    
    int zoneCount() const;
    
    void setMaxHistorySize(int max);
    int maxHistorySize() const;
    
    void clear();
    
    // Analytics
    double getAverageCongestion(int zoneId) const;
    double getPeakCongestion(int zoneId) const;
    bool isCongestionIncreasing(int zoneId) const;
    double predictCongestion(int zoneId, qint64 futureTime) const;
    
signals:
    void zoneUpdated(int zoneId);
    
private:
    mutable QMutex mutex_;
    QMap<int, ZoneHistory> zones_;
    int maxHistorySize_;
};