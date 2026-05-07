#pragma once

#include <QObject>
#include <QMutex>
#include <QMap>
#include <functional>

class SpatialFrame;

class StreamDispatcher : public QObject {
    Q_OBJECT
    
public:
    StreamDispatcher(QObject* parent = nullptr);
    ~StreamDispatcher();
    
    void initialize();
    
    void dispatchFrame(const SpatialFrame& frame);
    
    void registerStream(const QString& streamId, 
                        std::function<void(const SpatialFrame&)> callback);
    void unregisterStream(const QString& streamId);
    
    void setStreamEnabled(const QString& streamId, bool enabled);
    bool isStreamEnabled(const QString& streamId) const;
    
    QVector<QString> registeredStreams() const;
    
signals:
    void frameDispatched(const QString& streamId, const SpatialFrame& frame);
    
private:
    QMutex mutex_;
    QMap<QString, std::function<void(const SpatialFrame&)>> streams_;
    QMap<QString, bool> streamEnabled_;
};