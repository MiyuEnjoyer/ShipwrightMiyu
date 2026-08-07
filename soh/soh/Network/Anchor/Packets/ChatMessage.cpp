#include "soh/Network/Anchor/Anchor.h"
#include "soh/Network/Anchor/ChatLog.h"
#include <nlohmann/json.hpp>

extern "C" {
#include "variables.h"
#include "functions.h"
#include "sfx.h"
}

void Anchor::SendPacket_ChatMessage(const std::string& message) {
    if (!isConnected || message.empty()) {
        return;
    }

    nlohmann::json payload;
    payload["type"] = CHAT_MESSAGE;
    payload["message"] = message;
    SendJsonToRemote(payload);

    const std::string ownName = CVarGetString(CVAR_REMOTE_ANCHOR("Name"), "");

    float r = 0.8f, g = 1.0f, b = 0.8f;
    auto ownIt = clients.find(ownClientId);
    if (ownIt != clients.end()) {
        r = ownIt->second.color.r / 255.0f;
        g = ownIt->second.color.g / 255.0f;
        b = ownIt->second.color.b / 255.0f;
    }

    ChatLog::AddMessage(ownClientId, ownName, r, g, b, message);

    static const u16 chatSfxIds[] = {
        NA_SE_SY_MESSAGE_PASS,
        NA_SE_SY_CURSOR,
        NA_SE_SY_DECIDE,
        NA_SE_SY_FSEL_CURSOR,
        NA_SE_SY_FSEL_DECIDE_L, 
        NA_SE_SY_METRONOME
    };

    int sfxIndex = CVarGetInteger(CVAR_REMOTE_ANCHOR("ChatSfx"), 0);
    if (sfxIndex < 0 || sfxIndex >= (int)(sizeof(chatSfxIds) / sizeof(chatSfxIds[0]))) {
        sfxIndex = 0;
    }

    Audio_PlaySoundGeneral(chatSfxIds[sfxIndex],
                         &gSfxDefaultPos,
                         4,
                         &gSfxDefaultFreqAndVolScale,
                         &gSfxDefaultFreqAndVolScale,
                         &gSfxDefaultReverb);
}

void Anchor::HandlePacket_ChatMessage(nlohmann::json payload) {
    const uint32_t clientId = payload.at("clientId").get<uint32_t>();
    const std::string message = payload.at("message").get<std::string>();

    std::string senderName = "Unknown";
    float r = 1.0f, g = 1.0f, b = 1.0f;

    auto it = clients.find(clientId);
    if (it != clients.end()) {
        senderName = it->second.name;
        r = it->second.color.r / 255.0f;
        g = it->second.color.g / 255.0f;
        b = it->second.color.b / 255.0f;
    }

    ChatLog::AddMessage(clientId, senderName, r, g, b, message);

    static const u16 chatSfxIds[] = {
        NA_SE_SY_MESSAGE_PASS,
        NA_SE_SY_CURSOR,
        NA_SE_SY_DECIDE,
        NA_SE_SY_FSEL_CURSOR,
        NA_SE_SY_FSEL_DECIDE_L, 
        NA_SE_SY_METRONOME
    };

    int sfxIndex = CVarGetInteger(CVAR_REMOTE_ANCHOR("ChatSfx"), 0);
    if (sfxIndex < 0 || sfxIndex >= (int)(sizeof(chatSfxIds) / sizeof(chatSfxIds[0]))) {
        sfxIndex = 0;
    }

    Audio_PlaySoundGeneral(chatSfxIds[sfxIndex],
                         &gSfxDefaultPos,
                         4,
                         &gSfxDefaultFreqAndVolScale,
                         &gSfxDefaultFreqAndVolScale,
                         &gSfxDefaultReverb);
}