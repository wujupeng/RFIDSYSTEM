#include "tag_position_engine.h"

TagPositionEngine::TagPositionEngine() : mode_(LocalizationMode::RSSI_PHASE) {}

TagPositionEngine& TagPositionEngine::instance() {
    static TagPositionEngine instance;
    return instance;
}

void TagPositionEngine::setMode(LocalizationMode mode) {
    mode_ = mode;
}

void TagPositionEngine::addObservation(const TagObservation& observation) {
    std::lock_guard<std::mutex> lock(mutex_);
    observations_[observation.epc].push_back(observation);
    
    if (observations_[observation.epc].size() > 100) {
        observations_[observation.epc].erase(observations_[observation.epc].begin());
    }
}

void TagPositionEngine::addReader(const ReaderInfo& reader) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = std::find_if(readers_.begin(), readers_.end(),
        [&](const ReaderInfo& r) { return r.reader_id == reader.reader_id; });
    
    if (it != readers_.end()) {
        *it = reader;
    } else {
        readers_.push_back(reader);
    }
}

void TagPositionEngine::update() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    for (auto& pair : observations_) {
        const std::string& epc = pair.first;
        auto& obs_list = pair.second;
        
        if (obs_list.empty()) {
            continue;
        }
        
        TagPosition pos;
        
        switch (mode_) {
            case LocalizationMode::RSSI_ONLY: {
                auto prob = rssi_localizer_.localize(obs_list, readers_);
                pos.x = prob.mean_x;
                pos.y = prob.mean_y;
                pos.confidence = 0.5;
                break;
            }
            case LocalizationMode::PHASE_ONLY: {
                auto prob = phase_localizer_.localize(obs_list, readers_);
                pos.x = prob.mean_x;
                pos.y = prob.mean_y;
                pos.confidence = 0.6;
                break;
            }
            case LocalizationMode::RSSI_PHASE: {
                auto rssi_prob = rssi_localizer_.localize(obs_list, readers_);
                auto phase_prob = phase_localizer_.localize(obs_list, readers_);
                
                pos.x = (rssi_prob.mean_x + phase_prob.mean_x) / 2;
                pos.y = (rssi_prob.mean_y + phase_prob.mean_y) / 2;
                pos.confidence = 0.7;
                break;
            }
            case LocalizationMode::AOA: {
                if (readers_.size() >= 2) {
                    std::vector<AngleMeasurement> measurements;
                    
                    for (const auto& reader : readers_) {
                        auto obs_for_reader = obs_list;
                        auto it = std::remove_if(obs_for_reader.begin(), obs_for_reader.end(),
                            [&](const TagObservation& o) { return o.reader_id != reader.reader_id; });
                        obs_for_reader.erase(it, obs_for_reader.end());
                        
                        if (!obs_for_reader.empty()) {
                            double angle = aoa_solver_.solve(obs_for_reader);
                            
                            AngleMeasurement m;
                            m.reader_x = reader.x;
                            m.reader_y = reader.y;
                            m.angle_rad = angle;
                            m.confidence = 0.8;
                            
                            measurements.push_back(m);
                        }
                    }
                    
                    if (measurements.size() >= 2) {
                        pos = triangulation_engine_.fromAngles(measurements);
                    }
                }
                break;
            }
            case LocalizationMode::BEAMFORMING: {
                if (!obs_list.empty()) {
                    auto beams = beamforming_engine_.formBeams(obs_list);
                    
                    if (!beams.empty() && !readers_.empty()) {
                        auto& reader = readers_[0];
                        double distance = 5.0;
                        pos.x = reader.x + cos(beams[0].angle) * distance;
                        pos.y = reader.y + sin(beams[0].angle) * distance;
                        pos.confidence = beams[0].confidence;
                    }
                }
                break;
            }
        }
        
        auto it = trackers_.find(epc);
        if (it != trackers_.end()) {
            pos = it->second.update(pos.x, pos.y, 0.1);
        } else {
            KalmanTracker tracker;
            tracker.init(pos.x, pos.y);
            trackers_[epc] = tracker;
        }
    }
}

TagPosition TagPositionEngine::getPosition(const std::string& epc) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = trackers_.find(epc);
    if (it != trackers_.end()) {
        return it->second.getState();
    }
    
    auto obs_it = observations_.find(epc);
    if (obs_it != observations_.end() && !obs_it->second.empty()) {
        auto prob = rssi_localizer_.localize(obs_it->second, readers_);
        
        TagPosition pos;
        pos.x = prob.mean_x;
        pos.y = prob.mean_y;
        pos.confidence = 0.5;
        return pos;
    }
    
    return TagPosition();
}

SpatialProbability TagPositionEngine::getProbability(const std::string& epc) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto obs_it = observations_.find(epc);
    if (obs_it != observations_.end() && !obs_it->second.empty()) {
        return rssi_localizer_.localize(obs_it->second, readers_);
    }
    
    return SpatialProbability();
}

void TagPositionEngine::clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    observations_.clear();
    trackers_.clear();
}

size_t TagPositionEngine::getTrackedTagCount() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return trackers_.size();
}