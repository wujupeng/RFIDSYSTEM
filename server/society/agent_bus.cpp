#include "agent_bus.h"

AgentBus::AgentBus() {}

AgentBus& AgentBus::instance() {
    static AgentBus instance;
    return instance;
}

void AgentBus::sendMessage(const AgentMessage& message) {
    std::lock_guard<std::mutex> lock(mutex_);
    messages_.push_back(message);
    
    auto it = subscribers_.find(message.recipient);
    if (it != subscribers_.end()) {
        it->second(message);
    }
}

void AgentBus::broadcastMessage(const AgentMessage& message) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    for (const auto& subscriber : subscribers_) {
        AgentMessage broadcast_msg = message;
        broadcast_msg.recipient = subscriber.first;
        messages_.push_back(broadcast_msg);
        subscriber.second(broadcast_msg);
    }
}

std::vector<AgentMessage> AgentBus::getMessagesFor(const std::string& recipient) {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<AgentMessage> result;
    
    for (const auto& msg : messages_) {
        if (msg.recipient == recipient) {
            result.push_back(msg);
        }
    }
    
    return result;
}

void AgentBus::subscribe(const std::string& agent_name, 
                        std::function<void(const AgentMessage&)> callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    subscribers_[agent_name] = callback;
}

void AgentBus::unsubscribe(const std::string& agent_name) {
    std::lock_guard<std::mutex> lock(mutex_);
    subscribers_.erase(agent_name);
}