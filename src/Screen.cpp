#include "Screen.h"

#include <imgui.h>

void ScreenStack::pushScreen(std::unique_ptr<Screen> screen)
{
    Action action;
    action.kind = Action::Kind::Push;
    action.screen = std::move(screen);
    m_actions.push_back(std::move(action));
}

void ScreenStack::popScreen()
{
    Action action;
    action.kind = Action::Kind::Pop;
    m_actions.push_back(std::move(action));
}

void ScreenStack::changeScreen(std::unique_ptr<Screen> screen)
{
    Action action;
    action.kind = Action::Kind::Change;
    action.screen = std::move(screen);
    m_actions.push_back(std::move(action));
}

void ScreenStack::update()
{
    for (Action& action : m_actions) {
        switch (action.kind) {
            case Action::Kind::Push:
                m_screens.push(std::move(action.screen));
                break;

            case Action::Kind::Pop:
                m_screens.pop();
                break;

            case Action::Kind::Change:
                while (!m_screens.empty()) {
                    m_screens.pop();
                }
                m_screens.push(std::move(action.screen));
                break;
        }
    }
    m_actions.clear();
}

Screen& ScreenStack::peekScreen()
{
    return *m_screens.top();
}

bool ScreenStack::hasScreen() const
{
    return !m_screens.empty();
}

Screen::Screen(ScreenStack* screens)
    : m_pScreens(screens)
{
}

bool imguiBeginCustom(const char* name)
{
    ImVec2 windowSize(1280 / 4, 720 / 2);
    ImGui::SetNextWindowSize(windowSize, ImGuiCond_Always);
    ImGui::SetNextWindowPos({windowSize.x + windowSize.x * 4 / 8.0f, windowSize.y / 2},
                            ImGuiCond_Always);
    return ImGui::Begin(name, nullptr,
                        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
                            ImGuiWindowFlags_NoCollapse |
                            ImGuiWindowFlags_AlwaysAutoResize);
}

bool imguiButtonCustom(const char* text)
{
    ImGui::SetCursorPos({ImGui::GetCursorPosX() + 100, ImGui::GetCursorPosY() + 20});
    return ImGui::Button(text, {100, 50});
}
