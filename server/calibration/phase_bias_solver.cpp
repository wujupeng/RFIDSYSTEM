#include "phase_bias_solver.h"

namespace calibration {

PhaseBiasSolver& PhaseBiasSolver::instance() {
    static PhaseBiasSolver solver;
    return solver;
}

void PhaseBiasSolver::calibrateReader(uint64_t reader_id) {
    ReaderBias bias;
    bias.reader_id = reader_id;
    reader_biases_[reader_id] = bias;
}

void PhaseBiasSolver::calibrateAntenna(uint64_t reader_id, uint64_t antenna_id) {
    auto it = reader_biases_.find(reader_id);
    if (it == reader_biases_.end()) {
        calibrateReader(reader_id);
        it = reader_biases_.find(reader_id);
    }
    
    PhaseBias antenna_bias;
    antenna_bias.reader_id = reader_id;
    antenna_bias.antenna_id = antenna_id;
    antenna_bias.phase_offset_deg = 0.0;
    antenna_bias.phase_std_dev = 0.0;
    antenna_bias.amplitude_correction = 1.0;
    antenna_bias.last_calibrated_timestamp = 0;
    
    it->second.antenna_biases.push_back(antenna_bias);
}

double PhaseBiasSolver::correctPhase(uint64_t reader_id, uint64_t antenna_id, double raw_phase_deg) const {
    auto reader_it = reader_biases_.find(reader_id);
    if (reader_it == reader_biases_.end()) {
        return raw_phase_deg;
    }
    
    for (const auto& bias : reader_it->second.antenna_biases) {
        if (bias.antenna_id == antenna_id) {
            return raw_phase_deg - bias.phase_offset_deg;
        }
    }
    
    return raw_phase_deg;
}

double PhaseBiasSolver::correctAmplitude(uint64_t reader_id, uint64_t antenna_id, double raw_amplitude) const {
    auto reader_it = reader_biases_.find(reader_id);
    if (reader_it == reader_biases_.end()) {
        return raw_amplitude;
    }
    
    for (const auto& bias : reader_it->second.antenna_biases) {
        if (bias.antenna_id == antenna_id) {
            return raw_amplitude * bias.amplitude_correction;
        }
    }
    
    return raw_amplitude;
}

const PhaseBias* PhaseBiasSolver::getBias(uint64_t reader_id, uint64_t antenna_id) const {
    auto reader_it = reader_biases_.find(reader_id);
    if (reader_it == reader_biases_.end()) {
        return nullptr;
    }
    
    for (const auto& bias : reader_it->second.antenna_biases) {
        if (bias.antenna_id == antenna_id) {
            return &bias;
        }
    }
    
    return nullptr;
}

void PhaseBiasSolver::loadCalibration(const std::vector<ReaderBias>& biases) {
    for (const auto& bias : biases) {
        reader_biases_[bias.reader_id] = bias;
    }
}

std::vector<ReaderBias> PhaseBiasSolver::exportBiases() const {
    std::vector<ReaderBias> result;
    for (const auto& pair : reader_biases_) {
        result.push_back(pair.second);
    }
    return result;
}

void PhaseBiasSolver::solveBiasLeastSquares(uint64_t reader_id) {
    auto it = reader_biases_.find(reader_id);
    if (it == reader_biases_.end()) {
        return;
    }
}

void PhaseBiasSolver::validateBias(PhaseBias& bias) {
    if (bias.phase_offset_deg < -180.0) bias.phase_offset_deg += 360.0;
    if (bias.phase_offset_deg > 180.0) bias.phase_offset_deg -= 360.0;
}

} // namespace calibration