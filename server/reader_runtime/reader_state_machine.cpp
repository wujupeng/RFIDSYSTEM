#include "reader_state_machine.h"

namespace reader_runtime {

ReaderStateMachine& ReaderStateMachine::instance() {
    static ReaderStateMachine machine;
    return machine;
}

bool ReaderStateMachine::transition(ReaderState new_state, const std::string& reason) {
    if (!isValidTransition(current_state_, new_state)) {
        return false;
    }
    
    last_transition_.from = current_state_;
    last_transition_.to = new_state;
    last_transition_.timestamp = 0;
    last_transition_.reason = reason;
    
    current_state_ = new_state;
    return true;
}

ReaderState ReaderStateMachine::getCurrentState() const {
    return current_state_;
}

bool ReaderStateMachine::isOnline() const {
    return current_state_ == ReaderState::ONLINE || current_state_ == ReaderState::DEGRADED;
}

bool ReaderStateMachine::isHealthy() const {
    return current_state_ == ReaderState::ONLINE;
}

const StateTransition* ReaderStateMachine::getLastTransition() const {
    return &last_transition_;
}

void ReaderStateMachine::reset() {
    current_state_ = ReaderState::OFFLINE;
}

bool ReaderStateMachine::isValidTransition(ReaderState from, ReaderState to) const {
    switch (from) {
        case ReaderState::OFFLINE:
            return to == ReaderState::BOOTING;
        case ReaderState::BOOTING:
            return to == ReaderState::INITIALIZING || to == ReaderState::ERROR;
        case ReaderState::INITIALIZING:
            return to == ReaderState::ONLINE || to == ReaderState::ERROR;
        case ReaderState::ONLINE:
            return to == ReaderState::DEGRADED || to == ReaderState::MAINTENANCE || to == ReaderState::SHUTDOWN;
        case ReaderState::DEGRADED:
            return to == ReaderState::ONLINE || to == ReaderState::MAINTENANCE || to == ReaderState::ERROR;
        case ReaderState::MAINTENANCE:
            return to == ReaderState::ONLINE || to == ReaderState::SHUTDOWN;
        case ReaderState::ERROR:
            return to == ReaderState::BOOTING || to == ReaderState::SHUTDOWN;
        case ReaderState::SHUTDOWN:
            return to == ReaderState::BOOTING;
        default:
            return false;
    }
}

} // namespace reader_runtime