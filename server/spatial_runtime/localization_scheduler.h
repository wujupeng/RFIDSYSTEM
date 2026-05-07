#pragma once

#include "../spatial_positioning/spatial_types.h"
#include "localization_budget.h"
#include "localization_worker_pool.h"
#include "localization_pipeline.h"
#include <mutex>

class LocalizationScheduler {
public:
    static LocalizationScheduler& instance();
    
    void initialize();
    
    void shutdown();
    
    void tick();
    
    void scheduleObservation(const TagObservation& observation);
    
    TagPosition getPosition(const std::string& epc);
    
    size_t getTrackedCount() const;
    
    void setMaxTags(size_t max);
    
private:
    LocalizationScheduler();
    
    void processPending();
    
    void applyDegradation();
    
    LocalizationBudget budget_;
    LocalizationWorkerPool worker_pool_;
    LocalizationPipeline pipeline_;
    
    std::vector<TagObservation> pending_observations_;
    std::map<std::string, TagPosition> last_positions_;
    
    size_t max_tags_ = 10000;
    mutable std::mutex mutex_;
};