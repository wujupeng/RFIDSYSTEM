#pragma once

#include <cstdint>
#include <vector>
#include <map>

namespace spatial_drift {

enum class RecalibrationStatus {
    IDLE,
    PENDING,
    RUNNING,
    COMPLETED,
    FAILED
};

struct RecalibrationTask {
    uint64_t task_id;
    RecalibrationStatus status;
    uint64_t trigger_timestamp;
    uint64_t start_timestamp;
    uint64_t completion_timestamp;
    float progress;
    std::string reason;
};

class AutoRecalibrationEngine {
public:
    static AutoRecalibrationEngine& instance();
    
    void initialize();
    
    void tick();
    
    void triggerRecalibration(const std::string& reason);
    
    void scheduleRecalibration(uint64_t delay_ms, const std::string& reason);
    
    RecalibrationStatus getStatus() const;
    
    const RecalibrationTask* getCurrentTask() const;
    
    void cancelPendingRecalibration();
    
    bool isRecalibrating() const;
    
private:
    AutoRecalibrationEngine() = default;
    
    void runRecalibration();
    
    void completeRecalibration(bool success);
    
    RecalibrationStatus status_ = RecalibrationStatus::IDLE;
    RecalibrationTask current_task_;
    std::vector<RecalibrationTask> task_history_;
};

} // namespace spatial_drift