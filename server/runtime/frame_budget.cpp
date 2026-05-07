#include "frame_budget.h"
#include <mutex>

FrameBudgetTracker::FrameBudgetTracker()
    : frame_count_(0), warning_count_(0), drop_count_(0) {
    current_.reset();
    average_.reset();
}

FrameBudgetTracker& FrameBudgetTracker::instance() {
    static FrameBudgetTracker instance;
    return instance;
}

void FrameBudgetTracker::startFrame() {
    current_.reset();
    frame_start_ = std::chrono::high_resolution_clock::now();
}

void FrameBudgetTracker::endFrame() {
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration<double, std::milli>(end - frame_start_);
    current_.total_ms = duration.count();
    
    frame_count_++;
    
    const double alpha = 0.05;
    average_.collect_ms = alpha * current_.collect_ms + (1 - alpha) * average_.collect_ms;
    average_.heatmap_ms = alpha * current_.heatmap_ms + (1 - alpha) * average_.heatmap_ms;
    average_.congestion_ms = alpha * current_.congestion_ms + (1 - alpha) * average_.congestion_ms;
    average_.decision_ms = alpha * current_.decision_ms + (1 - alpha) * average_.decision_ms;
    average_.overlay_ms = alpha * current_.overlay_ms + (1 - alpha) * average_.overlay_ms;
    average_.upload_ms = alpha * current_.upload_ms + (1 - alpha) * average_.upload_ms;
    average_.render_ms = alpha * current_.render_ms + (1 - alpha) * average_.render_ms;
    average_.total_ms = alpha * current_.total_ms + (1 - alpha) * average_.total_ms;
    
    if (current_.total_ms > DROP_THRESHOLD_MS) {
        drop_count_++;
    } else if (current_.total_ms > WARNING_THRESHOLD_MS) {
        warning_count_++;
    }
}

void FrameBudgetTracker::recordCollect(double ms) {
    current_.collect_ms = ms;
}

void FrameBudgetTracker::recordHeatmap(double ms) {
    current_.heatmap_ms = ms;
}

void FrameBudgetTracker::recordCongestion(double ms) {
    current_.congestion_ms = ms;
}

void FrameBudgetTracker::recordDecision(double ms) {
    current_.decision_ms = ms;
}

void FrameBudgetTracker::recordOverlay(double ms) {
    current_.overlay_ms = ms;
}

void FrameBudgetTracker::recordUpload(double ms) {
    current_.upload_ms = ms;
}

void FrameBudgetTracker::recordRender(double ms) {
    current_.render_ms = ms;
}

const FrameBudget& FrameBudgetTracker::getCurrentBudget() const {
    return current_;
}

const FrameBudget& FrameBudgetTracker::getAverageBudget() const {
    return average_;
}

bool FrameBudgetTracker::isOverBudget() const {
    return current_.total_ms > WARNING_THRESHOLD_MS;
}

bool FrameBudgetTracker::shouldDropOptionalPasses() const {
    return current_.total_ms > DROP_THRESHOLD_MS;
}

uint64_t FrameBudgetTracker::getFrameCount() const {
    return frame_count_;
}

double FrameBudgetTracker::getFps() const {
    if (average_.total_ms <= 0.0) {
        return 0.0;
    }
    return 1000.0 / average_.total_ms;
}