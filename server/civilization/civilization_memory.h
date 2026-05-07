#pragma once

#include "event_timeline.h"
#include "evolution_history.h"
#include "law_evolution.h"
#include <memory>

class CivilizationMemory {
public:
    static CivilizationMemory& instance();
    
    void recordEvent(const TimelineEvent& event);
    
    void recordPolicyEvolution(const PolicyEvolution& evolution);
    
    void recordLawChange(const LawRevision& revision);
    
    void recordRecoverySuccess(uint64_t failure_type, double recovery_time_ms);
    
    void recordFactoryFailure(uint64_t factory_id);
    
    EventTimeline& getEventTimeline();
    
    EvolutionHistory& getEvolutionHistory();
    
    LawEvolution& getLawEvolution();
    
private:
    CivilizationMemory();
    
    EventTimeline event_timeline_;
    EvolutionHistory evolution_history_;
    LawEvolution law_evolution_;
};