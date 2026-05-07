#include "beamforming_engine.h"
#include <cmath>
#include <algorithm>

BeamformingEngine::BeamformingEngine() {}

double BeamformingEngine::calculateBeamPower(const std::vector<double>& phases, double target_angle) {
    if (phases.empty()) {
        return 0.0;
    }
    
    double sum_real = 0.0;
    double sum_imag = 0.0;
    double d = 0.05;
    double lambda = 0.122;
    
    for (size_t i = 0; i < phases.size(); ++i) {
        double steering = (2 * M_PI * i * d * sin(target_angle)) / lambda;
        double phase = phases[i] + steering;
        
        sum_real += cos(phase);
        sum_imag += sin(phase);
    }
    
    double magnitude = sqrt(sum_real * sum_real + sum_imag * sum_imag);
    return magnitude * magnitude / phases.size();
}

std::vector<Beam> BeamformingEngine::formBeams(const std::vector<TagObservation>& observations) {
    std::vector<Beam> beams;
    
    if (observations.empty()) {
        return beams;
    }
    
    std::vector<double> phases;
    for (const auto& obs : observations) {
        phases.push_back(obs.phase);
    }
    
    double max_power = 0.0;
    double best_angle = 0.0;
    
    for (double angle = -M_PI; angle <= M_PI; angle += 0.01) {
        double power = calculateBeamPower(phases, angle);
        
        if (power > max_power) {
            max_power = power;
            best_angle = angle;
        }
    }
    
    Beam beam;
    beam.angle = best_angle;
    beam.power = max_power;
    beam.confidence = std::min(1.0, max_power / 10.0);
    
    beams.push_back(beam);
    return beams;
}

SpatialEnergyMap BeamformingEngine::scanSpace(const std::vector<TagObservation>& observations,
                                             double min_angle,
                                             double max_angle,
                                             double resolution) {
    SpatialEnergyMap map;
    map.resolution = resolution;
    
    int num_points = static_cast<int>((max_angle - min_angle) / resolution);
    map.width = num_points;
    map.height = 1;
    map.energy.resize(1, std::vector<double>(num_points, 0.0));
    
    std::vector<double> phases;
    for (const auto& obs : observations) {
        phases.push_back(obs.phase);
    }
    
    for (int i = 0; i < num_points; ++i) {
        double angle = min_angle + i * resolution;
        map.energy[0][i] = calculateBeamPower(phases, angle);
    }
    
    return map;
}

TagPosition BeamformingEngine::findMaxEnergyPosition(const SpatialEnergyMap& map,
                                                    double origin_x,
                                                    double origin_y) {
    TagPosition pos;
    pos.confidence = 0.0;
    
    if (map.energy.empty()) {
        return pos;
    }
    
    double max_energy = 0.0;
    int max_idx = 0;
    
    for (int i = 0; i < map.width; ++i) {
        if (map.energy[0][i] > max_energy) {
            max_energy = map.energy[0][i];
            max_idx = i;
        }
    }
    
    double angle = max_idx * map.resolution - M_PI;
    double distance = 5.0;
    
    pos.x = origin_x + cos(angle) * distance;
    pos.y = origin_y + sin(angle) * distance;
    pos.confidence = std::min(1.0, max_energy / 10.0);
    
    return pos;
}