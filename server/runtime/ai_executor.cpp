#include "ai_executor.h"

AIExecutor::AIExecutor()
    : running_(false), enabled_(true),
      processed_count_(0), queue_size_(0) {
}

AIExecutor& AIExecutor::instance() {
    static AIExecutor instance;
    return instance;
}

void AIExecutor::initialize() {
    ai_analysis_pass_.initialize();
    trajectory_graph_.initialize(64, 10.0f);
    risk_field_.initialize(64, 10.0f);
    
    running_ = true;
    enabled_ = true;
    worker_thread_ = std::thread(&AIExecutor::processingLoop, this);
}

void AIExecutor::shutdown() {
    running_ = false;
    cv_.notify_all();
    
    if (worker_thread_.joinable()) {
        worker_thread_.join();
    }
    
    ai_analysis_pass_.shutdown();
    trajectory_graph_.shutdown();
    risk_field_.shutdown();
    
    std::lock_guard<std::mutex> lock(queue_mutex_);
    frame_queue_.clear();
}

void AIExecutor::submitFrame(std::shared_ptr<SpatialFrame> frame) {
    if (!enabled_) {
        return;
    }
    
    {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        frame_queue_.push_back(frame);
        queue_size_ = frame_queue_.size();
    }
    
    cv_.notify_one();
}

void AIExecutor::registerCallback(std::function<void(std::shared_ptr<AIFrameContext>)> callback) {
    std::lock_guard<std::mutex> lock(callback_mutex_);
    callbacks_.push_back(callback);
}

bool AIExecutor::hasPendingWork() const {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    return !frame_queue_.empty();
}

size_t AIExecutor::getQueueSize() const {
    return queue_size_;
}

void AIExecutor::setEnabled(bool enabled) {
    enabled_ = enabled;
    if (!enabled_) {
        cv_.notify_all();
    }
}

bool AIExecutor::isEnabled() const {
    return enabled_;
}

void AIExecutor::processingLoop() {
    while (running_) {
        std::shared_ptr<SpatialFrame> frame;
        
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            
            cv_.wait_for(lock, std::chrono::milliseconds(100), [this] {
                return !frame_queue_.empty() || !running_ || !enabled_;
            });
            
            if (!running_ || !enabled_) {
                continue;
            }
            
            if (!frame_queue_.empty()) {
                frame = frame_queue_.front();
                frame_queue_.erase(frame_queue_.begin());
                queue_size_ = frame_queue_.size();
            }
        }
        
        if (frame) {
            processFrame(frame);
        }
    }
}

void AIExecutor::processFrame(std::shared_ptr<SpatialFrame> frame) {
    auto context = analyzeFrame(*frame);
    
    {
        std::lock_guard<std::mutex> lock(callback_mutex_);
        for (const auto& callback : callbacks_) {
            try {
                callback(context);
            } catch (...) {
            }
        }
    }
    
    processed_count_++;
}

std::shared_ptr<AIFrameContext> AIExecutor::analyzeFrame(const SpatialFrame& frame) {
    std::vector<std::pair<uint64_t, float>> assetRisks;
    std::vector<std::pair<float, float>> assetPositions;
    
    for (const auto& asset : frame.assets) {
        trajectory_graph_.updateAssetPosition(asset.asset_id, asset.x, asset.y, frame.timestamp);
        
        assetRisks.push_back({static_cast<uint64_t>(asset.asset_id), static_cast<float>(asset.risk_score)});
        assetPositions.push_back({static_cast<float>(asset.x), static_cast<float>(asset.y)});
    }
    
    trajectory_graph_.finalizeFrame(frame.timestamp);
    
    risk_field_.updateFromAssets(assetRisks, assetPositions);
    risk_field_.computePropagation();
    
    const auto& riskMap = risk_field_.getRiskMap();
    
    std::vector<RiskCell> riskCells;
    for (const auto& cell : riskMap) {
        if (cell.risk_value > 0.01f || cell.propagation_risk > 0.01f) {
            riskCells.push_back(cell);
        }
    }
    
    std::vector<ZonePrediction> predictions;
    ai_analysis_pass_.analyze(riskCells, predictions);
    
    return std::make_shared<AIFrameContext>(ai_analysis_pass_.getContext());
}