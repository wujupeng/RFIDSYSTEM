#pragma once

#include <atomic>
#include <chrono>
#include <mutex>
#include <string>

class SpatialClock {
public:
    enum class ClockMode {
        REALTIME,
        PLAYBACK,
        SIMULATION
    };
    
    // Singleton access - all modules MUST use this
    static SpatialClock& instance();
    
    // Initialize clock
    void initialize();
    
    // Get current time (in seconds since epoch)
    double now() const;
    
    // Get current timestamp (in milliseconds since epoch)
    int64_t timestamp() const;
    
    // Clock mode management
    ClockMode mode() const;
    void setMode(ClockMode mode);
    
    // Playback control
    void setPlaybackTime(double time);
    void setPlaybackSpeed(double speed);
    double playbackSpeed() const;
    
    // Simulation control
    void setSimulationStartTime(double time);
    void setSimulationSpeed(double speed);
    
    // Tick - call this to advance time
    void tick();
    
    // Synchronize to external time source
    void synchronize();
    
    // For deterministic playback verification
    bool isDeterministic() const;
    
private:
    SpatialClock();
    ~SpatialClock();
    SpatialClock(const SpatialClock&) = delete;
    SpatialClock& operator=(const SpatialClock&) = delete;
    
    std::atomic<double> currentTime_;
    std::atomic<int64_t> currentTimestamp_;
    
    ClockMode mode_;
    mutable std::mutex modeMutex_;
    
    // Playback state
    double playbackTime_;
    double playbackSpeed_;
    mutable std::mutex playbackMutex_;
    
    // Simulation state
    double simulationStartTime_;
    double simulationSpeed_;
    double simulationElapsed_;
    mutable std::mutex simulationMutex_;
    
    // Real time state
    std::chrono::high_resolution_clock::time_point startTime_;
    
    // Deterministic mode flag
    std::atomic<bool> deterministic_;
};