#pragma once

#include <cstdint>
#include <chrono>
#include <string>

struct FrameBudget {
    double collect_ms = 0.0;
    double heatmap_ms = 0.0;
    double congestion_ms = 0.0;
    double decision_ms = 0.0;
    double overlay_ms = 0.0;
    double upload_ms = 0.0;
    double render_ms = 0.0;
    
    double total_ms = 0.0;
    
    void reset() {
        collect_ms = 0.0;
        heatmap_ms = 0.0;
        congestion_ms = 0.0;
        decision_ms = 0.0;
        overlay_ms = 0.0;
        upload_ms = 0.0;
        render_ms = 0.0;
        total_ms = 0.0;
    }
    
    void updateTotal() {
        total_ms = collect_ms + heatmap_ms + congestion_ms + 
                   decision_ms + overlay_ms + upload_ms + render_ms;
    }
};

class ScopedTimer {
public:
    ScopedTimer(const std::string& name, double* target)
        : name_(name), target_(target), 
          start_(std::chrono::high_resolution_clock::now()) {}
    
    ~ScopedTimer() {
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration<double, std::milli>(end - start_);
        *target_ = duration.count();
    }
    
private:
    std::string name_;
    double* target_;
    std::chrono::high_resolution_clock::time_point start_;
};

class FrameBudgetTracker {
public:
    static FrameBudgetTracker& instance();
    
    void startFrame();
    void endFrame();
    
    void recordCollect(double ms);
    void recordHeatmap(double ms);
    void recordCongestion(double ms);
    void recordDecision(double ms);
    void recordOverlay(double ms);
    void recordUpload(double ms);
    void recordRender(double ms);
    
    const FrameBudget& getCurrentBudget() const;
    const FrameBudget& getAverageBudget() const;
    
    bool isOverBudget() const;
    bool shouldDropOptionalPasses() const;
    
    uint64_t getFrameCount() const;
    double getFps() const;
    
private:
    FrameBudgetTracker();
    
    FrameBudget current_;
    FrameBudget average_;
    
    uint64_t frame_count_;
    uint64_t warning_count_;
    uint64_t drop_count_;
    
    std::chrono::high_resolution_clock::time_point frame_start_;
    
    static constexpr double WARNING_THRESHOLD_MS = 100.0;
    static constexpr double DROP_THRESHOLD_MS = 300.0;
};