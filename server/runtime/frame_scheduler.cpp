#include "frame_scheduler.h"
#include "spatial_frame.h"
#include "stream_dispatcher.h"
#include "frame_budget.h"
#include "linucb_bandit.h"
#include <chrono>
#include <thread>
#include <random>
#include <cmath>

FrameScheduler::FrameScheduler() 
    : running_(false), 
      frame_id_(0), 
      fps_(10),
      last_frame_time_(std::chrono::high_resolution_clock::now()),
      skipped_frames_(0),
      CELL_SIZE(10.0),  // 10m x 10m
      GRID_SIZE(64) {  // 64 x 64 grid
}

FrameScheduler::~FrameScheduler() {
    stop();
}

void FrameScheduler::start() {
    if (running_) return;
    
    running_ = true;
    frame_thread_ = std::thread(&FrameScheduler::frameLoop, this);
}

void FrameScheduler::stop() {
    running_ = false;
    if (frame_thread_.joinable()) {
        frame_thread_.join();
    }
}

void FrameScheduler::frameLoop() {
    const std::chrono::duration<double> frame_duration(1.0 / fps_);
    
    while (running_) {
        auto frame_start = std::chrono::high_resolution_clock::now();
        
        SpatialFrame frame;
        buildFrame(frame);
        
        // Broadcast to all streams
        if (dispatcher_) {
            dispatcher_->broadcast(frame);
        }
        
        frame_id_++;
        
        // Sleep until next frame
        auto frame_end = std::chrono::high_resolution_clock::now();
        auto elapsed = frame_end - frame_start;
        
        if (elapsed < frame_duration) {
            std::this_thread::sleep_for(frame_duration - elapsed);
        } else {
            skipped_frames_++;
            // Log frame drop
            // std::cout << "Frame " << frame_id_ << " skipped" << std::endl;
        }
        
        last_frame_time_ = std::chrono::high_resolution_clock::now();
    }
}

void FrameScheduler::buildFrame(SpatialFrame& frame) {
    auto& budgetTracker = FrameBudgetTracker::instance();
    budgetTracker.startFrame();
    
    frame.frame_id = frame_id_;
    frame.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::high_resolution_clock::now().time_since_epoch()).count();
    
    {
        ScopedTimer timer("collect", &frame.collect_ms);
        collectAssets(frame);
    }
    
    if (!budgetTracker.shouldDropOptionalPasses()) {
        ScopedTimer timer("heatmap", &frame.heatmap_ms);
        updateHeatmap(frame);
    }
    
    {
        ScopedTimer timer("congestion", &frame.congestion_ms);
        analyzeCongestion(frame);
    }
    
    {
        ScopedTimer timer("decision", &frame.decision_ms);
        appendBanditDecisions(frame);
    }
    
    {
        ScopedTimer timer("overlay", &frame.overlay_ms);
        appendDecisionOverlay(frame);
    }
    
    budgetTracker.recordCollect(frame.collect_ms);
    budgetTracker.recordHeatmap(frame.heatmap_ms);
    budgetTracker.recordCongestion(frame.congestion_ms);
    budgetTracker.recordDecision(frame.decision_ms);
    budgetTracker.recordOverlay(frame.overlay_ms);
    
    budgetTracker.endFrame();
    
    frame.processing_time_ms = budgetTracker.getCurrentBudget().total_ms;
}

void FrameScheduler::collectAssets(SpatialFrame& frame) {
    // Simulate RFID data collection
    // In real implementation, this would read from RFID readers
    
    static std::mt19937 rng(std::random_device{}());
    static std::uniform_real_distribution<double> pos_dist(0, 640);  // 640m x 640m area
    static std::uniform_real_distribution<double> vel_dist(-0.5, 0.5);
    static std::uniform_real_distribution<double> risk_dist(0, 1);
    
    // Simulate 100-200 devices
    int asset_count = 100 + (frame_id_ % 100);
    
    frame.assets.resize(asset_count);
    
    for (int i = 0; i < asset_count; ++i) {
        AssetPosition& asset = frame.assets[i];
        asset.asset_id = i + 1;
        asset.asset_name = "Asset-" + std::to_string(i + 1);
        asset.tag_id = "EPC-" + std::to_string(10000 + i);
        
        // Random position with slight movement
        static std::map<int, std::pair<double, double>> positions;
        if (!positions.count(i)) {
            positions[i] = {pos_dist(rng), pos_dist(rng)};
        }
        
        double& x = positions[i].first;
        double& y = positions[i].second;
        
        // Add some movement
        x += vel_dist(rng);
        y += vel_dist(rng);
        
        // Wrap around boundaries
        x = std::fmod(x + 640, 640);
        y = std::fmod(y + 640, 640);
        
        asset.x = x;
        asset.y = y;
        asset.vx = vel_dist(rng);
        asset.vy = vel_dist(rng);
        asset.risk_score = risk_dist(rng);
        asset.state = (asset.risk_score > 0.8) ? 2 : (asset.risk_score > 0.5) ? 1 : 0;
        asset.size = 8.0f;
        
        // Simple prediction: velocity extrapolation 3 seconds ahead
        asset.predicted_x = x + asset.vx * 3.0;
        asset.predicted_y = y + asset.vy * 3.0;
        asset.prediction_confidence = 0.7;
        
        // Bandit info
        asset.top_action = (asset.risk_score > 0.8) ? "INSPECT" : "NO_ACTION";
        asset.top_score = asset.risk_score;
    }
    
    frame.predictions.resize(asset_count);
    for (int i = 0; i < asset_count; ++i) {
        PathPrediction& pred = frame.predictions[i];
        pred.asset_id = i + 1;
        pred.x = frame.assets[i].predicted_x;
        pred.y = frame.assets[i].predicted_y;
        pred.confidence = 0.7;
        pred.timestamp = frame.timestamp + 3000; // 3 seconds ahead
    }
}

void FrameScheduler::updateHeatmap(SpatialFrame& frame) {
    // 64 x 64 grid, 10m per cell
    std::vector<std::vector<int>> grid(GRID_SIZE, std::vector<int>(GRID_SIZE, 0));
    
    for (const auto& asset : frame.assets) {
        int gx = static_cast<int>(asset.x / CELL_SIZE);
        int gy = static_cast<int>(asset.y / CELL_SIZE);
        
        if (gx >= 0 && gx < GRID_SIZE && gy >= 0 && gy < GRID_SIZE) {
            grid[gx][gy]++;
        }
    }
    
    // Convert to HeatCell
    frame.heatmap.clear();
    for (int x = 0; x < GRID_SIZE; ++x) {
        for (int y = 0; y < GRID_SIZE; ++y) {
            if (grid[x][y] > 0) {
                HeatCell cell;
                cell.x = x;
                cell.y = y;
                cell.value = static_cast<float>(grid[x][y]);
                frame.heatmap.push_back(cell);
            }
        }
    }
}

void FrameScheduler::analyzeCongestion(SpatialFrame& frame) {
    // 10m x 10m grid cells
    // HIGH_DENSITY: > 20 devices
    // CONGESTED: > 30 devices
    
    std::vector<std::vector<int>> grid(GRID_SIZE, std::vector<int>(GRID_SIZE, 0));
    
    for (const auto& asset : frame.assets) {
        int gx = static_cast<int>(asset.x / CELL_SIZE);
        int gy = static_cast<int>(asset.y / CELL_SIZE);
        
        if (gx >= 0 && gx < GRID_SIZE && gy >= 0 && gy < GRID_SIZE) {
            grid[gx][gy]++;
        }
    }
    
    frame.congestion.clear();
    int zone_id = 0;
    
    for (int x = 0; x < GRID_SIZE; ++x) {
        for (int y = 0; y < GRID_SIZE; ++y) {
            int count = grid[x][y];
            if (count > 20) {
                CongestionZone zone;
                zone.id = zone_id++;
                zone.name = "Zone-" + std::to_string(zone.id);
                zone.x = x * CELL_SIZE + CELL_SIZE / 2;
                zone.y = y * CELL_SIZE + CELL_SIZE / 2;
                zone.radius = CELL_SIZE / 2;
                zone.asset_count = count;
                zone.density = static_cast<float>(count) / 40.0f; // Normalized to 0-1
                
                if (count > 30) {
                    zone.level = "CONGESTED";
                } else {
                    zone.level = "HIGH_DENSITY";
                }
                
                frame.congestion.push_back(zone);
            }
        }
    }
}

void FrameScheduler::appendBanditDecisions(SpatialFrame& frame) {
    frame.decisions.clear();
    
    for (const auto& asset : frame.assets) {
        if (asset.risk_score > 0.7) {
            DecisionPoint decision;
            decision.asset_id = asset.asset_id;
            decision.action = asset.top_action;
            decision.confidence = asset.top_score;
            decision.expected_reward = 1.0 - asset.risk_score;
            decision.reason = (asset.risk_score > 0.8) ? "High risk score" : "Medium risk score";
            frame.decisions.push_back(decision);
        }
    }
}

void FrameScheduler::setDispatcher(StreamDispatcher* dispatcher) {
    dispatcher_ = dispatcher;
}

uint64_t FrameScheduler::currentFrameId() const {
    return frame_id_;
}

int FrameScheduler::skippedFrames() const {
    return skipped_frames_;
}