#pragma once

#include <QObject>
#include <QAbstractListModel>
#include <QVector>
#include <QMap>

enum class AssetState {
    NORMAL = 0,
    OBSERVE = 1,
    INSPECT = 2,
    SECURITY_ALERT = 3
};

struct AssetVisual {
    int id;
    QString name;
    QString tagId;
    float x;
    float y;
    float vx;
    float vy;
    float risk;
    AssetState state;
    float size;
    float heat;
    
    // Bandit info
    QMap<QString, double> actionScores;
    QString topAction;
    double topScore;
};

class AssetVisualModel : public QAbstractListModel {
    Q_OBJECT
    
public:
    enum Roles {
        IdRole = Qt::UserRole + 1,
        NameRole,
        TagIdRole,
        XRole,
        YRole,
        VXRole,
        VYRole,
        RiskRole,
        StateRole,
        SizeRole,
        HeatRole,
        TopActionRole,
        TopScoreRole
    };
    
    explicit AssetVisualModel(QObject* parent = nullptr);
    
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;
    
    void updateAsset(const AssetVisual& asset);
    void removeAsset(int id);
    void clear();
    const AssetVisual* getAsset(int id) const;
    
signals:
    void assetUpdated(int id);
    void assetRemoved(int id);
    
private:
    QVector<AssetVisual> assets;
    QMap<int, int> idToIndex;
};