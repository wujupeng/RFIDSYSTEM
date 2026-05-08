#pragma once

#include <cstdint>
#include <vector>
#include <map>
#include <array>

namespace calibration {

struct PhaseBias {
    uint64_t reader_id;
    uint64_t antenna_id;
    double phase_offset_deg;
    double phase_std_dev;
    double amplitude_correction;
    uint64_t last_calibrated_timestamp;
};

struct ReaderBias {
    uint64_t reader_id;
    std::vector<PhaseBias> antenna_biases;
};

class PhaseBiasSolver {
public:
    static PhaseBiasSolver& instance();
    
    void calibrateReader(uint64_t reader_id);
    
    void calibrateAntenna(uint64_t reader_id, uint64_t antenna_id);
    
    double correctPhase(uint64_t reader_id, uint64_t antenna_id, double raw_phase_deg) const;
    
    double correctAmplitude(uint64_t reader_id, uint64_t antenna_id, double raw_amplitude) const;
    
    const PhaseBias* getBias(uint64_t reader_id, uint64_t antenna_id) const;
    
    void loadCalibration(const std::vector<ReaderBias>& biases);
    
    std::vector<ReaderBias> exportBiases() const;
    
private:
    PhaseBiasSolver() = default;
    
    void solveBiasLeastSquares(uint64_t reader_id);
    
    void validateBias(PhaseBias& bias);
    
    std::map<uint64_t, ReaderBias> reader_biases_;
};

} // namespace calibration