#pragma once

#include <cstdint>
#include <vector>
#include <map>

namespace reader_runtime {

enum class AntennaState {
    ACTIVE,
    STANDBY,
    FAULT,
    CALIBRATING
};

struct AntennaInfo {
    uint64_t antenna_id;
    uint64_t reader_id;
    AntennaState state;
    float signal_strength;
    float noise_level;
    float temperature;
    uint64_t last_update_timestamp;
};

class AntennaRuntime {
public:
    static AntennaRuntime& instance();
    
    void addAntenna(uint64_t reader_id, uint64_t antenna_id);
    
    void removeAntenna(uint64_t reader_id, uint64_t antenna_id);
    
    void updateAntenna(uint64_t reader_id, uint64_t antenna_id, const AntennaInfo& info);
    
    const AntennaInfo* getAntenna(uint64_t reader_id, uint64_t antenna_id) const;
    
    std::vector<AntennaInfo> getAntennasForReader(uint64_t reader_id) const;
    
    int getActiveAntennaCount(uint64_t reader_id) const;
    
    void loadAntennas(const std::vector<AntennaInfo>& antennas);
    
private:
    AntennaRuntime() = default;
    
    std::map<std::pair<uint64_t, uint64_t>, AntennaInfo> antennas_;
};

} // namespace reader_runtime