#pragma once

#include <cstdint>
#include <vector>
#include <map>
#include <array>

namespace calibration {

struct RFGridCell {
    float x;
    float y;
    float attenuation;
    float multipath_intensity;
    float metal_reflection_prob;
    float rssi_bias;
    float reliability_score;
};

struct RFMap {
    uint64_t version;
    float grid_size_m;
    float min_x;
    float max_x;
    float min_y;
    float max_y;
    std::vector<RFGridCell> cells;
};

class RFMapBuilder {
public:
    static RFMapBuilder& instance();
    
    void buildMap(float min_x, float max_x, float min_y, float max_y, float grid_size);
    
    void updateCell(float x, float y, const RFGridCell& cell);
    
    void integrateObservation(float x, float y, float rssi, float phase_noise);
    
    const RFGridCell* getCell(float x, float y) const;
    
    float getAttenuation(float x, float y) const;
    
    float getMultipathIntensity(float x, float y) const;
    
    float getRSSIBias(float x, float y) const;
    
    const RFMap& getCurrentMap() const;
    
    void loadMap(const RFMap& map);
    
    void saveMap(const std::string& path) const;
    
private:
    RFMapBuilder() = default;
    
    size_t getCellIndex(float x, float y) const;
    
    RFMap current_map_;
};

} // namespace calibration