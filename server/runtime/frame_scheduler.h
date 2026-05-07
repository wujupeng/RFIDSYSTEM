#pragma once

#include <atomic>
#include <thread>
#include <memory>
#include <chrono>

class StreamDispatcher;
class SpatialFrame;

class FrameScheduler {
public:
    FrameScheduler();
    ~FrameScheduler();
    
    void start();
    void stop();
    
    void setDispatcher(StreamDispatcher* dispatcher);
    
    uint64_t currentFrameId() const;
    int skippedFrames() const;
    
private:
    void frameLoop();
    void buildFrame(SpatialFrame& frame);
    
    void collectAssets(SpatialFrame& frame);
    void updateHeatmap(SpatialFrame& frame);
    void analyzeCongestion(SpatialFrame& frame);
    void appendBanditDecisions(SpatialFrame& frame);
    
    std::atomic<bool> running_;
    std::atomic<uint64_t> frame_id_;
    int fps_;
    
    std::thread frame_thread_;
    std::chrono::high_resolution_clock::time_point last_frame_time_;
    
    std::atomic<int> skipped_frames_;
    
    StreamDispatcher* dispatcher_;
    
    // Constants
    static constexpr double CELL_SIZE = 10.0;
    static constexpr int GRID_SIZE = 64;
};