#include "ScreenMainMenu.h"

#include <imgui/imgui.h>

#include "ScreenInGame.h"

ScreenMainMenu::ScreenMainMenu(ScreenStack* stack)
    : Screen(stack)

{
}

void ScreenMainMenu::onRender()
{
    switch (m_activeMenu) {
        case ScreenMainMenu::Menu::MainMenu:
            showMainMenu();
            break;

        case ScreenMainMenu::Menu::SettingsMenu:
            showSettingsMenu();
            break;
    }
}

void ScreenMainMenu::showMainMenu()
{
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
}

void ScreenMainMenu::showSettingsMenu()
{
    if (imguiBeginCustom("S E T T I N G S")) {

        if (imguiButtonCustom("Back")) {
            m_activeMenu = Menu::MainMenu;
        }
    }
    ImGui::End();
}
