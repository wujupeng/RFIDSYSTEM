#pragma once

#include <cstdint>
#include <vector>
#include <map>

namespace reader_runtime {

struct HealthMetric {
    float temperature;
    float noise_floor;
    float packet_loss;
    float rf_power;
    float phase_stability;
    float antenna_health;
    uint64_t timestamp;
};

struct HealthScore {
    float overall;
    float temperature_score;
    float noise_score;
    float packet_loss_score;
    float power_score;
    float stability_score;
    uint64_t timestamp;
};

class ReaderHealthScore {
public:
    static ReaderHealthScore& instance();
    
    void updateMetrics(const HealthMetric& metrics);
    
    HealthScore computeScore();
    
    float getOverallHealth() const;
    
    bool isHealthy() const;
    
    bool needsMaintenance() const;
    
    const HealthMetric& getLatestMetrics() const;
    
    void reset();
    
private:
    ReaderHealthScore() = default;
    
    HealthMetric latest_metrics_;
    HealthScore current_score_;
};

} // namespace reader_runtime