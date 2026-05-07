#pragma once

#include <cstdint>
#include <string>

enum class GovernanceEventType {
    DECISION_SUBMITTED,
    DECISION_APPROVED,
    DECISION_REJECTED,
    CONSTITUTION_VIOLATION,
    PERMISSION_DENIED,
    SANDBOX_SIMULATED,
    ROLLBACK_EXECUTED,
    SNAPSHOT_CREATED,
    AUDIT_RECORDED
};

struct GovernanceEvent {
    GovernanceEventType type;
    uint64_t timestamp;
    uint64_t frame_id;
    std::string message;
    bool approved;
    std::string reason;
    
    GovernanceEvent() 
        : type(GovernanceEventType::DECISION_SUBMITTED), 
          timestamp(0), 
          frame_id(0),
          approved(false) {}
};