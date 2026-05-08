#include "calibration_snapshot.h"
#include "phase_bias_solver.h"
#include "rf_map_builder.h"

namespace calibration {

CalibrationSnapshotManager& CalibrationSnapshotManager::instance() {
    static CalibrationSnapshotManager manager;
    return manager;
}

void CalibrationSnapshotManager::createSnapshot(const std::string& description) {
    CalibrationSnapshot snapshot;
    snapshot.version = ++current_version_;
    snapshot.timestamp = 0;
    snapshot.description = description;
    snapshot.rf_map = RFMapBuilder::instance().getCurrentMap();
    snapshot.reader_biases = PhaseBiasSolver::instance().exportBiases();
    
    snapshots_[snapshot.version] = snapshot;
}

bool CalibrationSnapshotManager::loadSnapshot(uint64_t version) {
    auto it = snapshots_.find(version);
    if (it == snapshots_.end()) {
        return false;
    }
    
    applySnapshotToRuntime(it->second);
    current_version_ = version;
    return true;
}

bool CalibrationSnapshotManager::loadSnapshotByTimestamp(uint64_t timestamp) {
    uint64_t closest_version = 0;
    uint64_t closest_diff = UINT64_MAX;
    
    for (const auto& pair : snapshots_) {
        uint64_t diff = (pair.second.timestamp > timestamp) 
            ? pair.second.timestamp - timestamp 
            : timestamp - pair.second.timestamp;
        if (diff < closest_diff) {
            closest_diff = diff;
            closest_version = pair.first;
        }
    }
    
    if (closest_version > 0) {
        return loadSnapshot(closest_version);
    }
    
    return false;
}

const CalibrationSnapshot* CalibrationSnapshotManager::getSnapshot(uint64_t version) const {
    auto it = snapshots_.find(version);
    return (it != snapshots_.end()) ? &it->second : nullptr;
}

const CalibrationSnapshot* CalibrationSnapshotManager::getCurrentSnapshot() const {
    auto it = snapshots_.find(current_version_);
    return (it != snapshots_.end()) ? &it->second : nullptr;
}

std::vector<uint64_t> CalibrationSnapshotManager::listSnapshotVersions() const {
    std::vector<uint64_t> versions;
    for (const auto& pair : snapshots_) {
        versions.push_back(pair.first);
    }
    return versions;
}

void CalibrationSnapshotManager::deleteSnapshot(uint64_t version) {
    snapshots_.erase(version);
}

void CalibrationSnapshotManager::applySnapshotToRuntime(const CalibrationSnapshot& snapshot) {
    PhaseBiasSolver::instance().loadCalibration(snapshot.reader_biases);
    RFMapBuilder::instance().loadMap(snapshot.rf_map);
}

} // namespace calibration