#include "civilization_memory.h"

CivilizationMemory::CivilizationMemory() {}

CivilizationMemory& CivilizationMemory::instance() {
    static CivilizationMemory instance;
    return instance;
}

void CivilizationMemory::recordEvent(const TimelineEvent& event) {
    event_timeline_.addEvent(event);
}

void CivilizationMemory::recordPolicyEvolution(const PolicyEvolution& evolution) {
    evolution_history_.recordPolicyChange(evolution);
}

void CivilizationMemory::recordLawChange(const LawRevision& revision) {
    law_evolution_.recordLawChange(revision);
}

void CivilizationMemory::recordRecoverySuccess(uint64_t failure_type, double recovery_time_ms) {
    evolution_history_.recordRecoverySuccess(failure_type, recovery_time_ms);
}

void CivilizationMemory::recordFactoryFailure(uint64_t factory_id) {
    evolution_history_.recordFactoryFailure(factory_id);
}

EventTimeline& CivilizationMemory::getEventTimeline() {
    return event_timeline_;
}

EvolutionHistory& CivilizationMemory::getEvolutionHistory() {
    return evolution_history_;
}

LawEvolution& CivilizationMemory::getLawEvolution() {
    return law_evolution_;
}