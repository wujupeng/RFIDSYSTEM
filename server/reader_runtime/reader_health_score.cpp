#include "reader_health_score.h"

namespace reader_runtime {

ReaderHealthScore& ReaderHealthScore::instance() {
    static ReaderHealthScore score;
    return score;
}

void ReaderHealthScore::updateMetrics(const HealthMetric& metrics) {
    latest_metrics_ = metrics;
    computeScore();
}

HealthScore ReaderHealthScore::computeScore() {
    HealthScore score;
    score.timestamp = latest_metrics_.timestamp;
    
    score.temperature_score = (latest_metrics_.temperature < 70.0f) ? 1.0f : 
                              (latest_metrics_.temperature < 85.0f) ? 0.5f : 0.0f;
    
    score.noise_score = (latest_metrics_.noise_floor < -80.0f) ? 1.0f : 
                        (latest_metrics_.noise_floor < -70.0f) ? 0.5f : 0.0f;
    
    score.packet_loss_score = (latest_metrics_.packet_loss < 0.01f) ? 1.0f : 
                              (latest_metrics_.packet_loss < 0.05f) ? 0.5f : 0.0f;
    
    score.power_score = (latest_metrics_.rf_power > 25.0f) ? 1.0f : 
                        (latest_metrics_.rf_power > 20.0f) ? 0.5f : 0.0f;
    
    score.stability_score = (latest_metrics_.phase_stability > 0.9f) ? 1.0f : 
                           (latest_metrics_.phase_stability > 0.7f) ? 0.5f : 0.0f;
    
    score.overall = (score.temperature_score * 0.2f + 
                     score.noise_score * 0.2f + 
                     score.packet_loss_score * 0.2f + 
                     score.power_score * 0.2f + 
                     score.stability_score * 0.2f);
    
    current_score_ = score;
    return score;
}

float ReaderHealthScore::getOverallHealth() const {
    return current_score_.overall;
}

bool ReaderHealthScore::isHealthy() const {
    return current_score_.overall >= 0.7f;
}

bool ReaderHealthScore::needsMaintenance() const {
    return current_score_.overall < 0.4f;
}

const HealthMetric& ReaderHealthScore::getLatestMetrics() const {
    return latest_metrics_;
}

void ReaderHealthScore::reset() {
    latest_metrics_ = {};
    current_score_ = {};
}

} // namespace reader_runtime