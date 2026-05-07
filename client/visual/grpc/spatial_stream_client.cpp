#include "spatial_stream_client.h"
#include "../../../server/runtime/spatial_frame.h"
#include <chrono>
#include <thread>
#include <random>
#include <cmath>

SpatialStreamClient::SpatialStreamClient(QObject* parent) 
    : QObject(parent), connected_(false), running_(false) {
}

SpatialStreamClient::~SpatialStreamClient() {
    disconnect();
}

void SpatialStreamClient::connectToServer(const QString& address) {
    if (connected_) {
        disconnect();
    }
    
    serverAddress_ = address;
    connected_ = true;
    running_ = true;
    
    streamThread_ = std::thread(&SpatialStreamClient::streamLoop, this);
    
    emit connected();
}

void SpatialStreamClient::disconnect() {
    running_ = false;
    connected_ = false;
    
    if (streamThread_.joinable()) {
        streamThread_.join();
    }
    
    emit disconnected();
}

bool SpatialStreamClient::isConnected() const {
    return connected_;
}

void SpatialStreamClient::setFrameCallback(std::function<void(const SpatialFrame&)> callback) {
    QMutexLocker locker(&callbackMutex_);
    frameCallback_ = callback;
}

void SpatialStreamClient::streamLoop() {
    std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<double> pos_dist(0, 640);
    std::uniform_real_distribution<double> vel_dist(-0.5, 0.5);
    std::uniform_real_distribution<double> risk_dist(0, 1);
    
    std::map<int, std::pair<double, double>> positions;
    
    uint64_t frame_id = 0;
    
    while (running_) {
        SpatialFrame frame;
        frame.frame_id = frame_id++;
        frame.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::high_resolution_clock::now().time_since_epoch()).count();
        
        // Generate 100-200 assets
        int asset_count = 100 + (frame_id % 100);
        frame.assets.resize(asset_count);
        
        for (int i = 0; i < asset_count; ++i) {
            AssetPosition& asset = frame.assets[i];
            asset.asset_id = i + 1;
            asset.asset_name = "Asset-" + std::to_string(i + 1);
            asset.tag_id = "EPC-" + std::to_string(10000 + i);
            
            if (!positions.count(i)) {
                positions[i] = {pos_dist(rng), pos_dist(rng)};
            }
            
            double& x = positions[i].first;
            double& y = positions[i].second;
            
            x += vel_dist(rng);
            y += vel_dist(rng);
            
            x = std::fmod(x + 640, 640);
            y = std::fmod(y + 640, 640);
            
            asset.x = x;
            asset.y = y;
            asset.vx = vel_dist(rng);
            asset.vy = vel_dist(rng);
            asset.risk_score = risk_dist(rng);
            asset.state = (asset.risk_score > 0.8) ? 2 : (asset.risk_score > 0.5) ? 1 : 0;
            asset.size = 8.0f;
            
            asset.predicted_x = x + asset.vx * 3.0;
            asset.predicted_y = y + asset.vy * 3.0;
            asset.prediction_confidence = 0.7;
            
            asset.top_action = (asset.risk_score > 0.8) ? "INSPECT" : "NO_ACTION";
            asset.top_score = asset.risk_score;
        }
        
        // Update heatmap (64x64 grid)
        const int GRID_SIZE = 64;
        const double CELL_SIZE = 10.0;
        std::vector<std::vector<int>> grid(GRID_SIZE, std::vector<int>(GRID_SIZE, 0));
        
        for (const auto& asset : frame.assets) {
            int gx = static_cast<int>(asset.x / CELL_SIZE);
            int gy = static_cast<int>(asset.y / CELL_SIZE);
            
            if (gx >= 0 && gx < GRID_SIZE && gy >= 0 && gy < GRID_SIZE) {
                grid[gx][gy]++;
            }
        }
        
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
        
        // Update congestion
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
                    zone.density = static_cast<float>(count) / 40.0f;
                    zone.level = (count > 30) ? "CONGESTED" : "HIGH_DENSITY";
                    frame.congestion.push_back(zone);
                }
            }
        }
        
        // Emit signal
        emit frameReceived(frame);
        
        // Call callback if set
        QMutexLocker locker(&callbackMutex_);
        if (frameCallback_) {
            frameCallback_(frame);
        }
        
        // 10 FPS
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}