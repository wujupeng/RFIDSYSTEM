#include "rf_map_builder.h"

namespace calibration {

RFMapBuilder& RFMapBuilder::instance() {
    static RFMapBuilder builder;
    return builder;
}

void RFMapBuilder::buildMap(float min_x, float max_x, float min_y, float max_y, float grid_size) {
    current_map_.min_x = min_x;
    current_map_.max_x = max_x;
    current_map_.min_y = min_y;
    current_map_.max_y = max_y;
    current_map_.grid_size_m = grid_size;
    current_map_.version = 0;
    
    int cells_x = static_cast<int>((max_x - min_x) / grid_size) + 1;
    int cells_y = static_cast<int>((max_y - min_y) / grid_size) + 1;
    
    current_map_.cells.resize(cells_x * cells_y);
    
    for (int iy = 0; iy < cells_y; ++iy) {
        for (int ix = 0; ix < cells_x; ++ix) {
            size_t idx = iy * cells_x + ix;
            current_map_.cells[idx].x = min_x + ix * grid_size;
            current_map_.cells[idx].y = min_y + iy * grid_size;
            current_map_.cells[idx].attenuation = 1.0f;
            current_map_.cells[idx].multipath_intensity = 0.0f;
            current_map_.cells[idx].metal_reflection_prob = 0.0f;
            current_map_.cells[idx].rssi_bias = 0.0f;
            current_map_.cells[idx].reliability_score = 1.0f;
        }
    }
}

void RFMapBuilder::updateCell(float x, float y, const RFGridCell& cell) {
    size_t idx = getCellIndex(x, y);
    if (idx < current_map_.cells.size()) {
        current_map_.cells[idx] = cell;
    }
}

void RFMapBuilder::integrateObservation(float x, float y, float rssi, float phase_noise) {
    size_t idx = getCellIndex(x, y);
    if (idx >= current_map_.cells.size()) {
        return;
    }
    
    RFGridCell& cell = current_map_.cells[idx];
    
    float alpha = 0.1f;
    cell.multipath_intensity = (1 - alpha) * cell.multipath_intensity + alpha * phase_noise;
    cell.rssi_bias = (1 - alpha) * cell.rssi_bias + alpha * rssi;
}

const RFGridCell* RFMapBuilder::getCell(float x, float y) const {
    size_t idx = getCellIndex(x, y);
    if (idx < current_map_.cells.size()) {
        return &current_map_.cells[idx];
    }
    return nullptr;
}

float RFMapBuilder::getAttenuation(float x, float y) const {
    const RFGridCell* cell = getCell(x, y);
    return cell ? cell->attenuation : 1.0f;
}

float RFMapBuilder::getMultipathIntensity(float x, float y) const {
    const RFGridCell* cell = getCell(x, y);
    return cell ? cell->multipath_intensity : 0.0f;
}

float RFMapBuilder::getRSSIBias(float x, float y) const {
    const RFGridCell* cell = getCell(x, y);
    return cell ? cell->rssi_bias : 0.0f;
}

const RFMap& RFMapBuilder::getCurrentMap() const {
    return current_map_;
}

void RFMapBuilder::loadMap(const RFMap& map) {
    current_map_ = map;
}

void RFMapBuilder::saveMap(const std::string& path) const {
}

size_t RFMapBuilder::getCellIndex(float x, float y) const {
    int ix = static_cast<int>((x - current_map_.min_x) / current_map_.grid_size_m);
    int iy = static_cast<int>((y - current_map_.min_y) / current_map_.grid_size_m);
    
    int cells_x = static_cast<int>((current_map_.max_x - current_map_.min_x) / current_map_.grid_size_m) + 1;
    
    return iy * cells_x + ix;
}

} // namespace calibration