#pragma once

#include "spatial_types.h"
#include "rssi_localizer.h"
#include "phase_localizer.h"
#include "aoa_solver.h"
#include "triangulation_engine.h"
#include "kalman_tracker.h"
#include "spatial_filter.h"
#include "beamforming_engine.h"
#include <map>
#include <mutex>

enum class LocalizationMode {
    RSSI_ONLY,
    PHASE_ONLY,
    RSSI_PHASE,
    AOA,
    BEAMFORMING
};

class TagPositionEngine {
public:
    static TagPositionEngine& instance();
    
    void setMode(LocalizationMode mode);
    
    void addObservation(const TagObservation& observation);
    
    void addReader(const ReaderInfo& reader);
    
    TagPosition getPosition(const std::string& epc);
    
    SpatialProbability getProbability(const std::string& epc);
    
    void update();
    
    void clear();
    
    size_t getTrackedTagCount() const;
    
private:
    TagPositionEngine();
    
    LocalizationMode mode_;
    
    RSSILocalizer rssi_localizer_;
    PhaseLocalizer phase_localizer_;
    AoASolver aoa_solver_;
    TriangulationEngine triangulation_engine_;
    SpatialFilter spatial_filter_;
    BeamformingEngine beamforming_engine_;
    
    std::map<std::string, std::vector<TagObservation>> observations_;
    std::map<std::string, KalmanTracker> trackers_;
    std::vector<ReaderInfo> readers_;
    
    mutable std::mutex mutex_;
};