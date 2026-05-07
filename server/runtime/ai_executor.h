#pragma once

#include "spatial_frame.h"
#include "ai_analysis_context.h"
#include "trajectory_graph.h"
#include "risk_field.h"
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <vector>
#include <functional>

class AIExecutor {
public:
    static AIExecutor& instance();
    
    void initialize();
    void shutdown();
    
    void submitFrame(std::shared_ptr<SpatialFrame> frame);
    
    void registerCallback(std::function<void(std::shared_ptr<AIFrameContext>)> callback);
    
    bool hasPendingWork() const;
    size_t getQueueSize() const;
    
    void setEnabled(bool enabled);
    bool isEnabled() const;
    
private:
    AIExecutor();
    
    void processingLoop();
    void processFrame(std::shared_ptr<SpatialFrame> frame);
    
    std::shared_ptr<AIFrameContext> analyzeFrame(const SpatialFrame& frame);
    
    std::thread worker_thread_;
    std::atomic<bool> running_;
    std::atomic<bool> enabled_;
    
    std::vector<std::shared_ptr<SpatialFrame>> frame_queue_;
    mutable std::mutex queue_mutex_;
    std::condition_variable cv_;
    
    std::vector<std::function<void(std::shared_ptr<AIFrameContext>)>> callbacks_;
    mutable std::mutex callback_mutex_;
    
    AIAnalysisPass ai_analysis_pass_;
    TrajectoryGraph trajectory_graph_;
    RiskField risk_field_;
    
    std::atomic<size_t> processed_count_;
    std::atomic<size_t> queue_size_;
};