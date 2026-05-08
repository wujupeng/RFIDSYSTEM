#include "reader_digital_twin.h"

namespace reader_runtime {

ReaderDigitalTwin& ReaderDigitalTwin::instance() {
    static ReaderDigitalTwin twin;
    return twin;
}

void ReaderDigitalTwin::createTwin(uint64_t reader_id, const std::string& name) {
    ReaderTwin twin;
    twin.reader_id = reader_id;
    twin.name = name;
    twin.temperature = 0.0f;
    twin.noise_floor = 0.0f;
    twin.packet_loss = 0.0f;
    twin.rf_power = 0.0f;
    twin.phase_stability = 0.0f;
    twin.antenna_health = 0.0f;
    twin.state = ReaderState::OFFLINE;
    twin.last_update_timestamp = 0;
    
    twins_[reader_id] = twin;
}

void ReaderDigitalTwin::updateTwin(uint64_t reader_id, const ReaderTwin& twin) {
    twins_[reader_id] = twin;
}

void ReaderDigitalTwin::updateHealth(uint64_t reader_id, const HealthMetric& metrics) {
    auto it = twins_.find(reader_id);
    if (it != twins_.end()) {
        it->second.temperature = metrics.temperature;
        it->second.noise_floor = metrics.noise_floor;
        it->second.packet_loss = metrics.packet_loss;
        it->second.rf_power = metrics.rf_power;
        it->second.phase_stability = metrics.phase_stability;
        it->second.antenna_health = metrics.antenna_health;
        it->second.last_update_timestamp = metrics.timestamp;
    }
}

const ReaderTwin* ReaderDigitalTwin::getTwin(uint64_t reader_id) const {
    auto it = twins_.find(reader_id);
    return (it != twins_.end()) ? &it->second : nullptr;
}

std::vector<ReaderTwin> ReaderDigitalTwin::getAllTwins() const {
    std::vector<ReaderTwin> result;
    for (const auto& pair : twins_) {
        result.push_back(pair.second);
    }
    return result;
}

std::vector<ReaderTwin> ReaderDigitalTwin::getHealthyTwins() const {
    std::vector<ReaderTwin> result;
    for (const auto& pair : twins_) {
        if (pair.second.state == ReaderState::ONLINE && 
            pair.second.phase_stability > 0.7f &&
            pair.second.packet_loss < 0.05f) {
            result.push_back(pair.second);
        }
    }
    return result;
}

std::vector<ReaderTwin> ReaderDigitalTwin::getUnhealthyTwins() const {
    std::vector<ReaderTwin> result;
    for (const auto& pair : twins_) {
        if (pair.second.state == ReaderState::ERROR || 
            pair.second.state == ReaderState::DEGRADED ||
            pair.second.phase_stability < 0.5f ||
            pair.second.packet_loss > 0.1f) {
            result.push_back(pair.second);
        }
    }
    return result;
}

void ReaderDigitalTwin::removeTwin(uint64_t reader_id) {
    twins_.erase(reader_id);
}

void ReaderDigitalTwin::loadTwins(const std::vector<ReaderTwin>& twins) {
    for (const auto& twin : twins) {
        twins_[twin.reader_id] = twin;
    }
}

} // namespace reader_runtime