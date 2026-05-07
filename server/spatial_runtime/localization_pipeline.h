#pragma once

#include "../spatial_positioning/spatial_types.h"
#include <vector>

enum class PipelineStage {
    OBSERVATION,
    RSSI_PROCESSING,
    PHASE_PROCESSING,
    AOA_SOLVING,
    TRIANGULATION,
    KALMAN_FILTERING,
    OUTPUT
};

class LocalizationPipeline {
public:
    static LocalizationPipeline& instance();
    
    void process(const std::vector<TagObservation>& observations,
                 TagPosition& output);
    
    void setStageEnabled(PipelineStage stage, bool enabled);
    
    bool isStageEnabled(PipelineStage stage) const;
    
    void reset();
    
private:
    LocalizationPipeline();
    
    std::map<PipelineStage, bool> stages_;
};