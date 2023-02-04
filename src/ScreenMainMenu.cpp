#include "ScreenMainMenu.h"

#include <imgui.h>

#include "ScreenInGame.h"
#include "Settings.h"

ScreenMainMenu::ScreenMainMenu(ScreenStack* stack)
    : Screen(stack)

{
}

void ScreenMainMenu::onRender()
{
    switch (m_activeMenu) {
        case ScreenMainMenu::Menu::MainMenu:
            if (imguiBeginCustom("M A I N   M E N U")) {
                ImGui::Text("Welcome to Pong 3D by Hopson");
                ImGui::Separator();

                if (imguiButtonCustom("Start Game")) {
                    m_pScreens->pushScreen(std::make_unique<ScreenInGame>(m_pScreens));
                }
                if (imguiButtonCustom("Settings")) {
                    m_activeMenu = Menu::SettingsMenu;
                }
                if (imguiButtonCustom("Exit Game")) {
                    m_pScreens->popScreen();
                }
            }
            ImGui::End();
            break;

        case ScreenMainMenu::Menu::SettingsMenu:
            Settings::get().showSettingsMenu([&] { m_activeMenu = Menu::MainMenu; });
            break;
    }
}