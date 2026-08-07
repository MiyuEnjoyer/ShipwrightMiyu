#include "Anchor.h"
#include "ChatLog.h"
#include <libultraship/bridge/controllerbridge.h>
#include <chrono>
#include <ctime>

#define ANCHOR_CHAT_GAME_INPUT_BLOCK_ID 8675309

int AnchorChatWindow::InputTextCallback(ImGuiInputTextCallbackData* data) {
    AnchorChatWindow* self = static_cast<AnchorChatWindow*>(data->UserData);
    if (data->EventFlag == ImGuiInputTextFlags_CallbackHistory) {
        self->HandleHistoryNavigation(data, data->EventKey == ImGuiKey_UpArrow);
    }
    return 0;
}

void AnchorChatWindow::HandleHistoryNavigation(ImGuiInputTextCallbackData* data, bool up) {
    if (mSentHistory.empty()) {
        return;
    }

    if (up) {
        if (mHistoryPos == -1) {
            mHistoryPos = static_cast<int32_t>(mSentHistory.size()) - 1;
        } else if (mHistoryPos > 0) {
            mHistoryPos--;
        }
    } else {
        if (mHistoryPos != -1) {
            mHistoryPos++;
            if (mHistoryPos >= static_cast<int32_t>(mSentHistory.size())) {
                mHistoryPos = -1;
            }
        }
    }

    const std::string replacement = (mHistoryPos == -1) ? "" : mSentHistory[mHistoryPos];
    data->DeleteChars(0, data->BufTextLen);
    data->InsertChars(0, replacement.c_str());
}

void AnchorChatWindow::Draw() {
    if (CVarGetInteger(CVAR_WINDOW("AnchorChat"), 0) && !IsVisible()) {
        Show();
    }

    if (mCaptureFrames > 0) {
        mCaptureFrames--;
        if (mCaptureFrames == 0) {
            // genuinely enough to stop game from registering game inputs between chat messages yes it's stupid
            ControllerUnblockGameInput(ANCHOR_CHAT_GAME_INPUT_BLOCK_ID);
        }
    }
    if (Anchor::Instance && Anchor::Instance->isConnected) {
        const int hotkeyChoice = CVarGetInteger(CVAR_REMOTE_ANCHOR("ChatHotkey"), 0);
        const ImGuiKey key = (hotkeyChoice == 1) ? ImGuiKey_T : ImGuiKey_Enter;

        if (ImGui::IsKeyPressed(key, false)) {
            const bool inputFocused = ImGui::GetIO().WantTextInput;
            const bool bufferEmpty = (mInputBuffer[0] == '\0');

            if (IsVisible() && inputFocused && bufferEmpty) {
                ImGui::SetWindowFocus(nullptr); 
                mRequestFocus = false;
                mCaptureFrames = 1;
                ControllerBlockGameInput(ANCHOR_CHAT_GAME_INPUT_BLOCK_ID);
            } else if (!inputFocused) {
                if (!IsVisible()) {
                    Show();
                    CVarSetInteger(CVAR_WINDOW("AnchorChat"), 1);
                    Ship::Context::GetInstance()->GetWindow()->GetGui()->SaveConsoleVariablesNextFrame();
                }
                mRequestFocus = true;
                mCaptureFrames = 1;
                ControllerBlockGameInput(ANCHOR_CHAT_GAME_INPUT_BLOCK_ID);
            }
        }
    }

    if (!IsVisible()) {
            return;
    }

    ImGui::SetNextWindowSize(ImVec2(420, 320), ImGuiCond_FirstUseEver);

    bool open = true;
    if (ImGui::Begin("Anchor Chat", &open, ImGuiWindowFlags_NoFocusOnAppearing)) {
        DrawElement();
    }
    ImGui::End();

    if (!open) {
        Hide();
        CVarSetInteger(CVAR_WINDOW("AnchorChat"), 0);
        Ship::Context::GetInstance()->GetWindow()->GetGui()->SaveConsoleVariablesNextFrame();
    }
}

void AnchorChatWindow::DrawElement() {
    const float footerHeight = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();

    ImGui::BeginChild("AnchorChatScrollback", ImVec2(0, -footerHeight), true);

    const bool showTimestamps = CVarGetInteger(CVAR_REMOTE_ANCHOR("ChatShowTimestamps"), 0) != 0;

    for (const ChatEntry& entry : ChatLog::GetMessages()) {
        if (showTimestamps) {
            auto time_t = std::chrono::system_clock::to_time_t(entry.timestamp);
            std::tm tm = *std::localtime(&time_t);

            char buf[16];
            std::strftime(buf, sizeof(buf), "[%H:%M] ", &tm);

            ImGui::TextDisabled("%s", buf);
            ImGui::SameLine(0.0f, 0.0f);
        }

        if (entry.isSystem) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(entry.colorR, entry.colorG, entry.colorB, 1.0f));
            ImGui::TextWrapped("* %s", entry.message.c_str());
            ImGui::PopStyleColor();
            continue;
        }

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(entry.colorR, entry.colorG, entry.colorB, 1.0f));
        ImGui::TextWrapped("%s:", entry.senderName.c_str());
        ImGui::PopStyleColor();
        ImGui::SameLine(0.0f, ImGui::GetStyle().ItemSpacing.x);
        ImGui::TextWrapped("%s", entry.message.c_str());
    }

    const bool wasAtBottom = ImGui::GetScrollY() >= ImGui::GetScrollMaxY() - 1.0f;
    if (wasAtBottom) {
        ImGui::SetScrollHereY(1.0f);
    }

    ImGui::EndChild();
    ImGui::Separator();

    const ImGuiInputTextFlags inputFlags =
        ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackHistory;

    if (mRequestFocus) {
        ImGui::SetKeyboardFocusHere(0);
        mRequestFocus = false;
    }

    ImGui::SetNextItemWidth(-1);
    const bool submitted = ImGui::InputText("##AnchorChatInput", mInputBuffer, IM_ARRAYSIZE(mInputBuffer),
    inputFlags, &AnchorChatWindow::InputTextCallback, this);

    if (submitted) {
        std::string message(mInputBuffer);

        if (!message.empty() && Anchor::Instance->isConnected) {
            Anchor::Instance->SendPacket_ChatMessage(message);
            mSentHistory.push_back(message);
            mHistoryPos = -1;
        }

        mInputBuffer[0] = '\0';
        mRequestFocus = true;
        mCaptureFrames = 1;
        ControllerBlockGameInput(ANCHOR_CHAT_GAME_INPUT_BLOCK_ID);
    }

    if (ImGui::IsItemActive() || submitted) {
        ImGui::GetIO().WantCaptureKeyboard = true;
    }
}