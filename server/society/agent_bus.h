#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <functional>
#include <mutex>

enum class MessageType {
    REQUEST,
    RESPONSE,
    NOTIFICATION,
    VOTE,
    PROPOSAL
};

struct AgentMessage {
    uint64_t message_id;
    std::string sender;
    std::string recipient;
    MessageType type;
    std::string content;
    uint64_t timestamp;
};

class AgentBus {
public:
    static AgentBus& instance();
    
    void sendMessage(const AgentMessage& message);
    
    void broadcastMessage(const AgentMessage& message);
    
    std::vector<AgentMessage> getMessagesFor(const std::string& recipient);
    
    void subscribe(const std::string& agent_name, 
                   std::function<void(const AgentMessage&)> callback);
    
    void unsubscribe(const std::string& agent_name);
    
private:
    AgentBus();
    
    std::vector<AgentMessage> messages_;
    std::map<std::string, std::function<void(const AgentMessage&)>> subscribers_;
    mutable std::mutex mutex_;
};