#include "heatmap_builder.h"
#include <cmath>

namespace pa::spatial_truth {

Heatmap HeatmapBuilder::build(const std::vector<RFEnvironmentData>& rf_data, double resolution,
                               double min_x, double max_x, double min_y, double max_y) const {
    Heatmap heatmap;
    heatmap.resolution = resolution;

    if (rf_data.empty()) return heatmap;

    int cols = static_cast<int>((max_x - min_x) / resolution) + 1;
    int rows = static_cast<int>((max_y - min_y) / resolution) + 1;
    if (cols <= 0 || rows <= 0) return heatmap;

    heatmap.grid.assign(rows, std::vector<double>(cols, 0.0));

    std::vector<std::vector<int>> counts(rows, std::vector<int>(cols, 0));

    for (const auto& d : rf_data) {
        int col = static_cast<int>((d.x - min_x) / resolution);
        int row = static_cast<int>((d.y - min_y) / resolution);
        if (col >= 0 && col < cols && row >= 0 && row < rows) {
            heatmap.grid[row][col] += d.rssi;
            counts[row][col]++;
        }
    }

    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            if (counts[r][c] > 0) {
                heatmap.grid[r][c] /= counts[r][c];
            }
        }
    }

    return heatmap;
}

std::vector<AoAVector> AoAVectorBuilder::build(const std::vector<AoAObservation>& observations) const {
    std::vector<AoAVector> vectors;
    for (const auto& obs : observations) {
        AoAVector v;
        v.reader_id = obs.reader_id;
        v.origin_x = obs.reader_x;
        v.origin_y = obs.reader_y;
        v.direction_x = std::cos(obs.angle);
        v.direction_y = std::sin(obs.angle);
        vectors.push_back(v);
    }
    return vectors;
}

} // namespace pa::spatial_truth