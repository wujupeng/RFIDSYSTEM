#pragma once

#include <QObject>
#include <QString>
#include <QMutex>
#include <atomic>
#include <memory>
#include <functional>

class SpatialFrame;

class SpatialStreamClient : public QObject {
    Q_OBJECT
    
public:
    explicit SpatialStreamClient(QObject* parent = nullptr);
    ~SpatialStreamClient();
    
    void connectToServer(const QString& address = "localhost:50051");
    void disconnect();
    
    bool isConnected() const;
    
    void setFrameCallback(std::function<void(const SpatialFrame&)> callback);
    
signals:
    void connected();
    void disconnected();
    void frameReceived(const SpatialFrame& frame);
    void errorOccurred(const QString& error);
    
private:
    void streamLoop();
    
    std::atomic<bool> connected_;
    std::atomic<bool> running_;
    
    QString serverAddress_;
    std::thread streamThread_;
    
    std::function<void(const SpatialFrame&)> frameCallback_;
    QMutex callbackMutex_;
};