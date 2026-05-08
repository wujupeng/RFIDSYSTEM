#pragma once

#include <cstdint>

namespace reader_runtime {

enum class ReaderState {
    OFFLINE,
    BOOTING,
    INITIALIZING,
    ONLINE,
    DEGRADED,
    MAINTENANCE,
    ERROR,
    SHUTDOWN
};

struct StateTransition {
    ReaderState from;
    ReaderState to;
    uint64_t timestamp;
    std::string reason;
};

class ReaderStateMachine {
public:
    static ReaderStateMachine& instance();
    
    bool transition(ReaderState new_state, const std::string& reason);
    
    ReaderState getCurrentState() const;
    
    bool isOnline() const;
    
    bool isHealthy() const;
    
    const StateTransition* getLastTransition() const;
    
    void reset();
    
private:
    ReaderStateMachine() = default;
    
    bool isValidTransition(ReaderState from, ReaderState to) const;
    
    ReaderState current_state_ = ReaderState::OFFLINE;
    StateTransition last_transition_;
};

} // namespace reader_runtime