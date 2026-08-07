#include "ChatLog.h"

std::deque<ChatEntry> ChatLog::messages;

void ChatLog::AddMessage(uint32_t clientId, const std::string& senderName, float r, float g, float b,
                          const std::string& message) {
    ChatEntry entry;
    entry.clientId   = clientId;
    entry.senderName = senderName;
    entry.colorR     = r;
    entry.colorG     = g;
    entry.colorB     = b;
    entry.message    = message;
    entry.isSystem   = false;
    entry.timestamp  = std::chrono::system_clock::now();

    messages.push_back(entry);

    while (messages.size() > MAX_HISTORY) {
        messages.pop_front();
    }
}

void ChatLog::AddSystemMessage(const std::string& message) {
    ChatEntry entry;
    entry.clientId   = 0;
    entry.senderName = "";
    entry.colorR     = 0.6f;
    entry.colorG     = 0.6f;
    entry.colorB     = 0.6f;
    entry.message    = message;
    entry.isSystem   = true;
    entry.timestamp  = std::chrono::system_clock::now();

    messages.push_back(entry);

    while (messages.size() > MAX_HISTORY) {
        messages.pop_front();
    }
}

const std::deque<ChatEntry>& ChatLog::GetMessages() {
    return messages;
}

void ChatLog::Clear() {
    messages.clear();
}