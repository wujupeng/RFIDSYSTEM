#include "antenna_runtime.h"

namespace reader_runtime {

AntennaRuntime& AntennaRuntime::instance() {
    static AntennaRuntime runtime;
    return runtime;
}

void AntennaRuntime::addAntenna(uint64_t reader_id, uint64_t antenna_id) {
    AntennaInfo info;
    info.antenna_id = antenna_id;
    info.reader_id = reader_id;
    info.state = AntennaState::STANDBY;
    info.signal_strength = 0.0f;
    info.noise_level = 0.0f;
    info.temperature = 0.0f;
    info.last_update_timestamp = 0;
    
    antennas_[{reader_id, antenna_id}] = info;
}

void AntennaRuntime::removeAntenna(uint64_t reader_id, uint64_t antenna_id) {
    antennas_.erase({reader_id, antenna_id});
}

void AntennaRuntime::updateAntenna(uint64_t reader_id, uint64_t antenna_id, const AntennaInfo& info) {
    antennas_[{reader_id, antenna_id}] = info;
}

const AntennaInfo* AntennaRuntime::getAntenna(uint64_t reader_id, uint64_t antenna_id) const {
    auto it = antennas_.find({reader_id, antenna_id});
    return (it != antennas_.end()) ? &it->second : nullptr;
}

std::vector<AntennaInfo> AntennaRuntime::getAntennasForReader(uint64_t reader_id) const {
    std::vector<AntennaInfo> result;
    for (const auto& pair : antennas_) {
        if (pair.first.first == reader_id) {
            result.push_back(pair.second);
        }
    }
    return result;
}

int AntennaRuntime::getActiveAntennaCount(uint64_t reader_id) const {
    int count = 0;
    for (const auto& pair : antennas_) {
        if (pair.first.first == reader_id && pair.second.state == AntennaState::ACTIVE) {
            count++;
        }
    }
    return count;
}

void AntennaRuntime::loadAntennas(const std::vector<AntennaInfo>& antennas) {
    for (const auto& antenna : antennas) {
        antennas_[{antenna.reader_id, antenna.antenna_id}] = antenna;
    }
}

} // namespace reader_runtime