#pragma once

#include <cstdint>
#include <vector>
#include <map>
#include "reader_state_machine.h"
#include "reader_health_score.h"
#include "antenna_runtime.h"

namespace reader_runtime {

struct ReaderTwin {
    uint64_t reader_id;
    std::string name;
    float temperature;
    float noise_floor;
    float packet_loss;
    float rf_power;
    float phase_stability;
    float antenna_health;
    ReaderState state;
    uint64_t last_update_timestamp;
};

class ReaderDigitalTwin {
public:
    static ReaderDigitalTwin& instance();
    
    void createTwin(uint64_t reader_id, const std::string& name);
    
    void updateTwin(uint64_t reader_id, const ReaderTwin& twin);
    
    void updateHealth(uint64_t reader_id, const HealthMetric& metrics);
    
    const ReaderTwin* getTwin(uint64_t reader_id) const;
    
    std::vector<ReaderTwin> getAllTwins() const;
    
    std::vector<ReaderTwin> getHealthyTwins() const;
    
    std::vector<ReaderTwin> getUnhealthyTwins() const;
    
    void removeTwin(uint64_t reader_id);
    
    void loadTwins(const std::vector<ReaderTwin>& twins);
    
private:
    ReaderDigitalTwin() = default;
    
    std::map<uint64_t, ReaderTwin> twins_;
};

} // namespace reader_runtime