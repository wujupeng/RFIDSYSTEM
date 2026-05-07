#pragma once

#include <QObject>
#include <QAbstractListModel>
#include <QVector>

struct HeatCell {
    int x;
    int y;
    float value;
    float normalizedValue;
};

struct CongestionZone {
    int id;
    QString name;
    float x;
    float y;
    float radius;
    float score;
    QString status;
    int deviceCount;
    float avgStayTime;
    float inflowRate;
    float outflowRate;
};

class HeatmapModel : public QAbstractListModel {
    Q_OBJECT
    
public:
    enum HeatCellRoles {
        XRole = Qt::UserRole + 1,
        YRole,
        ValueRole,
        NormalizedRole
    };
    
    enum CongestionRoles {
        ZoneIdRole = Qt::UserRole + 1,
        ZoneNameRole,
        ZoneXRole,
        ZoneYRole,
        ZoneRadiusRole,
        ZoneScoreRole,
        ZoneStatusRole,
        DeviceCountRole,
        AvgStayRole,
        InflowRole,
        OutflowRole
    };
    
    explicit HeatmapModel(QObject* parent = nullptr);
    
    // Heatmap cells
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;
    
    void updateHeatmap(const QVector<HeatCell>& cells);
    void clear();
    
    // Congestion zones
    int congestionCount() const;
    const CongestionZone& congestionAt(int index) const;
    void updateCongestion(const QVector<CongestionZone>& zones);
    void clearCongestion();
    
signals:
    void heatmapUpdated();
    void congestionUpdated();
    
private:
    QVector<HeatCell> heatCells;
    QVector<CongestionZone> congestionZones;
    float maxValue;
    
    QString getStatusFromScore(float score) const;
};