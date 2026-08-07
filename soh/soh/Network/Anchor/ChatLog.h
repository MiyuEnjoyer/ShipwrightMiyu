#ifndef NETWORK_ANCHOR_CHATLOG_H
#define NETWORK_ANCHOR_CHATLOG_H
#ifdef __cplusplus

#include <deque>
#include <string>
#include <cstdint>
#include <chrono>

struct ChatEntry {
    uint32_t clientId; 
    std::string senderName;
    float colorR;
    float colorG;
    float colorB;
    std::string message;
    bool isSystem;
    std::chrono::system_clock::time_point timestamp;
};

class ChatLog {
  public:
    static constexpr size_t MAX_HISTORY = 2000;

    static void AddMessage(uint32_t clientId, const std::string& senderName, float r, float g, float b,
                            const std::string& message);
    static void AddSystemMessage(const std::string& message);
    static const std::deque<ChatEntry>& GetMessages();
    static void Clear();

  private:
    static std::deque<ChatEntry> messages;
};

#endif 
#endif
