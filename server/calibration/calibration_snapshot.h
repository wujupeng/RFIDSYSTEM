#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include "rf_map_builder.h"
#include "phase_bias_solver.h"

namespace calibration {

struct CalibrationSnapshot {
    uint64_t version;
    uint64_t timestamp;
    RFMap rf_map;
    std::vector<ReaderBias> reader_biases;
    std::string description;
};

class CalibrationSnapshotManager {
public:
    static CalibrationSnapshotManager& instance();
    
    void createSnapshot(const std::string& description);
    
    bool loadSnapshot(uint64_t version);
    
    bool loadSnapshotByTimestamp(uint64_t timestamp);
    
    const CalibrationSnapshot* getSnapshot(uint64_t version) const;
    
    const CalibrationSnapshot* getCurrentSnapshot() const;
    
    std::vector<uint64_t> listSnapshotVersions() const;
    
    void deleteSnapshot(uint64_t version);
    
    void applySnapshotToRuntime(const CalibrationSnapshot& snapshot);
    
private:
    CalibrationSnapshotManager() = default;
    
    std::map<uint64_t, CalibrationSnapshot> snapshots_;
    uint64_t current_version_ = 0;
};

} // namespace calibration