#pragma once

#include <QObject>
#include <QString>
#include <QDateTime>
#include "../../../server/runtime/spatial_frame.h"
#include <vector>
#include <memory>

class ReplayController : public QObject {
    Q_OBJECT
    
    Q_PROPERTY(bool isReplaying READ isReplaying NOTIFY isReplayingChanged)
    Q_PROPERTY(double progress READ progress NOTIFY progressChanged)
    Q_PROPERTY(QDateTime currentTime READ currentTime NOTIFY currentTimeChanged)
    Q_PROPERTY(bool hashValid READ hashValid NOTIFY hashValidChanged)
    Q_PROPERTY(uint64_t currentHash READ currentHash NOTIFY currentHashChanged)
    Q_PROPERTY(uint64_t recordedHash READ recordedHash NOTIFY recordedHashChanged)
    
public:
    explicit ReplayController(QObject* parent = nullptr);
    
    bool isReplaying() const;
    double progress() const;
    QDateTime currentTime() const;
    bool hashValid() const;
    uint64_t currentHash() const;
    uint64_t recordedHash() const;
    
    Q_INVOKABLE void startReplay(const QString& filePath);
    Q_INVOKABLE void stopReplay();
    Q_INVOKABLE void pauseReplay();
    Q_INVOKABLE void resumeReplay();
    Q_INVOKABLE void seekTo(double progress);
    
    void processFrame(const SpatialFrame& frame);
    
signals:
    void isReplayingChanged();
    void progressChanged();
    void currentTimeChanged();
    void hashValidChanged();
    void currentHashChanged();
    void recordedHashChanged();
    void frameReady(const SpatialFrame& frame);
    
private:
    bool loadRecording(const QString& filePath);
    void validateDecisionHash(const SpatialFrame& frame);
    
    bool is_replaying_;
    bool is_paused_;
    double progress_;
    QDateTime current_time_;
    bool hash_valid_;
    uint64_t current_hash_;
    uint64_t recorded_hash_;
    
    std::vector<SpatialFrame> recorded_frames_;
    size_t current_frame_index_;
    
    bool use_recorded_decisions_;
};