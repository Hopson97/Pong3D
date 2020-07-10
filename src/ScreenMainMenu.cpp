#include "ScreenMainMenu.h"

#include <imgui/imgui.h>

#include "ScreenInGame.h"

ScreenMainMenu::ScreenMainMenu(ScreenStack* stack)
    : Screen(stack)

{
}

void ScreenMainMenu::onInput()
{
}

void ScreenMainMenu::onUpdate([[maybe_unused]]float dt)
{
    
}

void ScreenMainMenu::onRender()
{
    ImVec2 windowSize(1280 / 4, 720 / 2);
    ImGui::SetNextWindowSize(windowSize, ImGuiCond_Always);
    ImGui::SetNextWindowPos({windowSize.x + windowSize.x * 4 / 8.0f, windowSize.y / 2},
                            ImGuiCond_Always);

    // Render GUI Stuff
    auto flags =
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse;
    if (imguiBeginCustom("M A I N   M E N U")) {
        ImGui::Text("Welcome to Pong 3D by Hopson");
        ImGui::Separator();

        if (imguiButtonCustom("Start Game")) {
            m_pScreens->pushScreen(std::make_unique<ScreenInGame>(m_pScreens));
        }
        if (imguiButtonCustom("Settings")) {
        }
        if (imguiButtonCustom("Exit Game")) {
            m_pScreens->popScreen();
        }
    }
    ImGui::End();
}
