#include "localization_pipeline.h"
#include "../spatial_positioning/rssi_localizer.h"
#include "../spatial_positioning/phase_localizer.h"
#include "../spatial_positioning/aoa_solver.h"
#include "../spatial_positioning/triangulation_engine.h"
#include "../spatial_positioning/kalman_tracker.h"
#include "localization_budget.h"

LocalizationPipeline::LocalizationPipeline() {
    stages_[PipelineStage::OBSERVATION] = true;
    stages_[PipelineStage::RSSI_PROCESSING] = true;
    stages_[PipelineStage::PHASE_PROCESSING] = true;
    stages_[PipelineStage::AOA_SOLVING] = true;
    stages_[PipelineStage::TRIANGULATION] = true;
    stages_[PipelineStage::KALMAN_FILTERING] = true;
    stages_[PipelineStage::OUTPUT] = true;
}

LocalizationPipeline& LocalizationPipeline::instance() {
    static LocalizationPipeline instance;
    return instance;
}

void LocalizationPipeline::process(const std::vector<TagObservation>& observations,
                                   TagPosition& output) {
    auto& budget = LocalizationBudget::instance();
    
    if (!stages_[PipelineStage::OBSERVATION] || observations.empty()) {
        return;
    }
    
    if (stages_[PipelineStage::RSSI_PROCESSING] && budget.hasBudget("rssi")) {
        RSSILocalizer rssi;
        auto prob = rssi.localize(observations, {});
        output.x = prob.mean_x;
        output.y = prob.mean_y;
    }
    
    if (stages_[PipelineStage::PHASE_PROCESSING] && budget.hasBudget("phase")) {
        PhaseLocalizer phase;
        auto prob = phase.localize(observations, {});
        output.x = (output.x + prob.mean_x) / 2;
        output.y = (output.y + prob.mean_y) / 2;
    }
    
    if (stages_[PipelineStage::AOA_SOLVING] && budget.hasBudget("aoa")) {
        AoASolver aoa;
        double angle = aoa.solve(observations);
    }
    
    if (stages_[PipelineStage::KALMAN_FILTERING] && budget.hasBudget("kalman")) {
        KalmanTracker tracker;
        tracker.init(output.x, output.y);
        output = tracker.update(output.x, output.y);
    }
    
    output.confidence = 0.7;
}

void LocalizationPipeline::setStageEnabled(PipelineStage stage, bool enabled) {
    stages_[stage] = enabled;
}

bool LocalizationPipeline::isStageEnabled(PipelineStage stage) const {
    auto it = stages_.find(stage);
    return it != stages_.end() && it->second;
}

void LocalizationPipeline::reset() {
    for (auto& pair : stages_) {
        pair.second = true;
    }
}