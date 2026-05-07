#include "stream_dispatcher.h"
#include "spatial_frame.h"

StreamDispatcher::StreamDispatcher(QObject* parent) 
    : QObject(parent) {
}

StreamDispatcher::~StreamDispatcher() {
}

void StreamDispatcher::initialize() {
    // Initialize default streams
    registerStream("qt_client", [this](const SpatialFrame& frame) {
        emit frameDispatched("qt_client", frame);
    });
    
    registerStream("web_client", [this](const SpatialFrame& frame) {
        emit frameDispatched("web_client", frame);
    });
    
    registerStream("debug", [this](const SpatialFrame& frame) {
        emit frameDispatched("debug", frame);
    });
}

void StreamDispatcher::dispatchFrame(const SpatialFrame& frame) {
    QMutexLocker locker(&mutex_);
    
    for (auto it = streams_.begin(); it != streams_.end(); ++it) {
        if (streamEnabled_[it.key()]) {
            it.value()(frame);
        }
    }
}

void StreamDispatcher::registerStream(const QString& streamId, 
                                     std::function<void(const SpatialFrame&)> callback) {
    QMutexLocker locker(&mutex_);
    streams_[streamId] = callback;
    streamEnabled_[streamId] = true;
}

void StreamDispatcher::unregisterStream(const QString& streamId) {
    QMutexLocker locker(&mutex_);
    streams_.remove(streamId);
    streamEnabled_.remove(streamId);
}

void StreamDispatcher::setStreamEnabled(const QString& streamId, bool enabled) {
    QMutexLocker locker(&mutex_);
    if (streamEnabled_.contains(streamId)) {
        streamEnabled_[streamId] = enabled;
    }
}

bool StreamDispatcher::isStreamEnabled(const QString& streamId) const {
    QMutexLocker locker(const_cast<QMutex*>(&mutex_));
    return streamEnabled_.value(streamId, false);
}

QVector<QString> StreamDispatcher::registeredStreams() const {
    QMutexLocker locker(const_cast<QMutex*>(&mutex_));
    return streams_.keys().toVector();
}