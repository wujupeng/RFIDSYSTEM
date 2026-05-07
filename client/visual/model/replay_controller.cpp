#include "replay_controller.h"
#include <QFile>
#include <QDataStream>

ReplayController::ReplayController(QObject* parent)
    : QObject(parent),
      is_replaying_(false),
      is_paused_(false),
      progress_(0.0),
      hash_valid_(true),
      current_hash_(0),
      recorded_hash_(0),
      current_frame_index_(0),
      use_recorded_decisions_(true) {
}

bool ReplayController::isReplaying() const {
    return is_replaying_;
}

double ReplayController::progress() const {
    return progress_;
}

QDateTime ReplayController::currentTime() const {
    return current_time_;
}

bool ReplayController::hashValid() const {
    return hash_valid_;
}

uint64_t ReplayController::currentHash() const {
    return current_hash_;
}

uint64_t ReplayController::recordedHash() const {
    return recorded_hash_;
}

void ReplayController::startReplay(const QString& filePath) {
    if (loadRecording(filePath)) {
        is_replaying_ = true;
        is_paused_ = false;
        current_frame_index_ = 0;
        progress_ = 0.0;
        
        if (!recorded_frames_.empty()) {
            processFrame(recorded_frames_[0]);
        }
        
        emit isReplayingChanged();
        emit progressChanged();
    }
}

void ReplayController::stopReplay() {
    is_replaying_ = false;
    is_paused_ = false;
    current_frame_index_ = 0;
    progress_ = 0.0;
    recorded_frames_.clear();
    
    emit isReplayingChanged();
    emit progressChanged();
}

void ReplayController::pauseReplay() {
    is_paused_ = true;
}

void ReplayController::resumeReplay() {
    is_paused_ = false;
}

void ReplayController::seekTo(double progress) {
    if (recorded_frames_.empty()) {
        return;
    }
    
    progress_ = qBound(0.0, progress, 1.0);
    current_frame_index_ = static_cast<size_t>(progress_ * (recorded_frames_.size() - 1));
    
    if (current_frame_index_ < recorded_frames_.size()) {
        processFrame(recorded_frames_[current_frame_index_]);
    }
    
    emit progressChanged();
}

void ReplayController::processFrame(const SpatialFrame& frame) {
    if (use_recorded_decisions_) {
        SpatialFrame recordedFrame = frame;
        
        if (current_frame_index_ < recorded_frames_.size()) {
            recordedFrame.overlays = recorded_frames_[current_frame_index_].overlays;
            recordedFrame.decision_hash = recorded_frames_[current_frame_index_].decision_hash;
        }
        
        validateDecisionHash(recordedFrame);
        
        emit frameReady(recordedFrame);
    } else {
        validateDecisionHash(frame);
        emit frameReady(frame);
    }
    
    current_time_ = QDateTime::fromMSecsSinceEpoch(static_cast<qint64>(frame.timestamp));
    emit currentTimeChanged();
}

bool ReplayController::loadRecording(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }
    
    QDataStream in(&file);
    in.setVersion(QDataStream::Qt_5_15);
    
    recorded_frames_.clear();
    
    while (!in.atEnd()) {
        SpatialFrame frame;
        in >> frame.timestamp;
        in >> frame.frame_id;
        
        size_t assetCount;
        in >> assetCount;
        frame.assets.resize(assetCount);
        for (size_t i = 0; i < assetCount; ++i) {
            in >> frame.assets[i].asset_id;
            in >> frame.assets[i].x;
            in >> frame.assets[i].y;
            in >> frame.assets[i].vx;
            in >> frame.assets[i].vy;
            in >> frame.assets[i].risk_score;
            in >> frame.assets[i].size;
        }
        
        size_t overlayCount;
        in >> overlayCount;
        frame.overlays.resize(overlayCount);
        for (size_t i = 0; i < overlayCount; ++i) {
            in >> frame.overlays[i].asset_id;
            in >> frame.overlays[i].x;
            in >> frame.overlays[i].y;
            in >> frame.overlays[i].action;
            in >> frame.overlays[i].confidence;
            in >> frame.overlays[i].missing_risk;
            in >> frame.overlays[i].abnormal_risk;
            in >> frame.overlays[i].inactivity_risk;
            
            char reason[128];
            in.readRawData(reason, 128);
            memcpy(frame.overlays[i].reason, reason, 128);
            
            in >> frame.overlays[i].timestamp;
        }
        
        in >> frame.decision_hash;
        
        recorded_frames_.push_back(frame);
    }
    
    return true;
}

void ReplayController::validateDecisionHash(const SpatialFrame& frame) {
    current_hash_ = frame.hash();
    recorded_hash_ = frame.decision_hash;
    
    hash_valid_ = (current_hash_ == recorded_hash_);
    
    emit currentHashChanged();
    emit recordedHashChanged();
    emit hashValidChanged();
}