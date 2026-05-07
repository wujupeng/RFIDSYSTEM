#include "spatial_runtime.h"
#include "frame_scheduler.h"
#include "stream_dispatcher.h"
#include "gpu_sync_manager.h"
#include "spatial_clock.h"
#include "spatial_frame.h"

SpatialRuntime::SpatialRuntime(QObject* parent) 
    : QObject(parent), running_(false), paused_(false), frameId_(0) {
    frameTimer_ = new QTimer(this);
    connect(frameTimer_, &QTimer::timeout, this, &SpatialRuntime::onFrameReady);
}

SpatialRuntime::~SpatialRuntime() {
    stop();
}

SpatialRuntime& SpatialRuntime::instance() {
    static SpatialRuntime instance;
    return instance;
}

void SpatialRuntime::initialize() {
    frameScheduler_ = QSharedPointer<FrameScheduler>::create();
    streamDispatcher_ = QSharedPointer<StreamDispatcher>::create();
    gpuSyncManager_ = QSharedPointer<GPUSyncManager>::create();
    spatialClock_ = QSharedPointer<SpatialClock>::create();
    
    frameScheduler_->initialize();
    streamDispatcher_->initialize();
    gpuSyncManager_->initialize();
    spatialClock_->initialize();
}

void SpatialRuntime::start() {
    if (running_) return;
    
    running_ = true;
    paused_ = false;
    
    frameTimer_->start(33); // 30 FPS
    
    emit runtimeStarted();
}

void SpatialRuntime::stop() {
    if (!running_) return;
    
    running_ = false;
    paused_ = false;
    
    frameTimer_->stop();
    
    emit runtimeStopped();
}

void SpatialRuntime::pause() {
    if (!running_ || paused_) return;
    
    paused_ = true;
    frameTimer_->stop();
    
    emit runtimePaused();
}

void SpatialRuntime::resume() {
    if (!running_ || !paused_) return;
    
    paused_ = false;
    frameTimer_->start(33);
    
    emit runtimeResumed();
}

bool SpatialRuntime::isRunning() const {
    return running_;
}

bool SpatialRuntime::isPaused() const {
    return paused_;
}

qint64 SpatialRuntime::currentFrameId() const {
    return frameId_;
}

double SpatialRuntime::currentTime() const {
    return spatialClock_->currentTime();
}

void SpatialRuntime::registerFrameCallback(std::function<void(const SpatialFrame&)> callback) {
    QMutexLocker locker(&callbackMutex_);
    frameCallbacks_.append(callback);
}

void SpatialRuntime::unregisterFrameCallback(std::function<void(const SpatialFrame&)> callback) {
    QMutexLocker locker(&callbackMutex_);
    frameCallbacks_.removeAll(callback);
}

FrameScheduler* SpatialRuntime::frameScheduler() const {
    return frameScheduler_.data();
}

StreamDispatcher* SpatialRuntime::streamDispatcher() const {
    return streamDispatcher_.data();
}

GPUSyncManager* SpatialRuntime::gpuSyncManager() const {
    return gpuSyncManager_.data();
}

SpatialClock* SpatialRuntime::spatialClock() const {
    return spatialClock_.data();
}

void SpatialRuntime::onFrameReady() {
    if (!running_ || paused_) return;
    
    frameId_++;
    
    SpatialFrame frame = frameScheduler_->buildFrame(frameId_, spatialClock_->currentTime());
    
    // Dispatch to all registered callbacks
    QMutexLocker locker(&callbackMutex_);
    for (const auto& callback : frameCallbacks_) {
        callback(frame);
    }
    
    emit frameReady(frame);
    
    // Sync with GPU
    gpuSyncManager_->syncFrame(frame);
}