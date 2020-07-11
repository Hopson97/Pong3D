#include "ScreenMainMenu.h"

#include <imgui/imgui.h>

#include "ScreenInGame.h"

ScreenMainMenu::ScreenMainMenu(ScreenStack* stack)
    : Screen(stack)

{
}

void ScreenMainMenu::onRender()
{
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
