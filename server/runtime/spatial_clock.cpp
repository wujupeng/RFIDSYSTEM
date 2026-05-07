#include "spatial_clock.h"

SpatialClock::SpatialClock() 
    : mode_(ClockMode::REALTIME), 
      playbackTime_(0.0), playbackSpeed_(1.0),
      simulationStartTime_(0.0), simulationSpeed_(1.0),
      simulationElapsed_(0.0), deterministic_(false) {
    startTime_ = std::chrono::high_resolution_clock::now();
    currentTime_ = 0.0;
    currentTimestamp_ = 0;
}

SpatialClock::~SpatialClock() {
}

SpatialClock& SpatialClock::instance() {
    static SpatialClock instance;
    return instance;
}

void SpatialClock::initialize() {
    synchronize();
}

double SpatialClock::now() const {
    return currentTime_;
}

int64_t SpatialClock::timestamp() const {
    return currentTimestamp_;
}

SpatialClock::ClockMode SpatialClock::mode() const {
    std::lock_guard<std::mutex> lock(modeMutex_);
    return mode_;
}

void SpatialClock::setMode(ClockMode mode) {
    std::lock_guard<std::mutex> lock(modeMutex_);
    
    if (mode_ != mode) {
        mode_ = mode;
        
        if (mode_ == ClockMode::REALTIME) {
            startTime_ = std::chrono::high_resolution_clock::now();
            simulationElapsed_ = 0.0;
            deterministic_ = false;
        } else if (mode_ == ClockMode::PLAYBACK) {
            deterministic_ = true;
        } else if (mode_ == ClockMode::SIMULATION) {
            simulationStartTime_ = currentTime_;
            simulationElapsed_ = 0.0;
            deterministic_ = true;
        }
    }
}

void SpatialClock::setPlaybackTime(double time) {
    std::lock_guard<std::mutex> lock(playbackMutex_);
    playbackTime_ = time;
    currentTime_ = time;
    currentTimestamp_ = static_cast<int64_t>(time * 1000);
}

void SpatialClock::setPlaybackSpeed(double speed) {
    std::lock_guard<std::mutex> lock(playbackMutex_);
    playbackSpeed_ = std::max(0.1, std::min(16.0, speed));
}

double SpatialClock::playbackSpeed() const {
    std::lock_guard<std::mutex> lock(playbackMutex_);
    return playbackSpeed_;
}

void SpatialClock::setSimulationStartTime(double time) {
    std::lock_guard<std::mutex> lock(simulationMutex_);
    simulationStartTime_ = time;
}

void SpatialClock::setSimulationSpeed(double speed) {
    std::lock_guard<std::mutex> lock(simulationMutex_);
    simulationSpeed_ = speed;
}

void SpatialClock::tick() {
    std::lock_guard<std::mutex> lock(modeMutex_);
    
    switch (mode_) {
        case ClockMode::REALTIME: {
            auto now = std::chrono::high_resolution_clock::now();
            auto elapsed = std::chrono::duration<double>(now - startTime_);
            currentTime_ = elapsed.count();
            currentTimestamp_ = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
            break;
        }
        
        case ClockMode::PLAYBACK: {
            std::lock_guard<std::mutex> playbackLock(playbackMutex_);
            playbackTime_ += 0.1 * playbackSpeed_; // 10 FPS tick
            currentTime_ = playbackTime_;
            currentTimestamp_ = static_cast<int64_t>(playbackTime_ * 1000);
            break;
        }
        
        case ClockMode::SIMULATION: {
            std::lock_guard<std::mutex> simLock(simulationMutex_);
            simulationElapsed_ += 0.1; // 10 FPS tick
            currentTime_ = simulationStartTime_ + simulationElapsed_ * simulationSpeed_;
            currentTimestamp_ = static_cast<int64_t>(currentTime_ * 1000);
            break;
        }
    }
}

void SpatialClock::synchronize() {
    tick();
}

bool SpatialClock::isDeterministic() const {
    return deterministic_;
}