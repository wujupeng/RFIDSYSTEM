#pragma once

#include <QObject>
#include <QTimer>
#include <QMutex>
#include <QSharedPointer>
#include <atomic>
#include <functional>

class FrameScheduler;
class StreamDispatcher;
class GPUSyncManager;
class SpatialClock;
class SpatialFrame;

class SpatialRuntime : public QObject {
    Q_OBJECT
    
public:
    ~SpatialRuntime();
    
    static SpatialRuntime& instance();
    
    void initialize();
    void start();
    void stop();
    void pause();
    void resume();
    
    bool isRunning() const;
    bool isPaused() const;
    
    qint64 currentFrameId() const;
    double currentTime() const;
    
    void registerFrameCallback(std::function<void(const SpatialFrame&)> callback);
    void unregisterFrameCallback(std::function<void(const SpatialFrame&)> callback);
    
    FrameScheduler* frameScheduler() const;
    StreamDispatcher* streamDispatcher() const;
    GPUSyncManager* gpuSyncManager() const;
    SpatialClock* spatialClock() const;
    
signals:
    void frameReady(const SpatialFrame& frame);
    void runtimeStarted();
    void runtimeStopped();
    void runtimePaused();
    void runtimeResumed();
    
private:
    SpatialRuntime(QObject* parent = nullptr);
    Q_DISABLE_COPY(SpatialRuntime)
    
    void onFrameReady();
    
    std::atomic<bool> running_;
    std::atomic<bool> paused_;
    std::atomic<qint64> frameId_;
    
    QSharedPointer<FrameScheduler> frameScheduler_;
    QSharedPointer<StreamDispatcher> streamDispatcher_;
    QSharedPointer<GPUSyncManager> gpuSyncManager_;
    QSharedPointer<SpatialClock> spatialClock_;
    
    QVector<std::function<void(const SpatialFrame&)>> frameCallbacks_;
    QMutex callbackMutex_;
    
    QTimer* frameTimer_;
};