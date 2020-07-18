#pragma once

#include "Screen.h"

class ScreenMainMenu final : public Screen {
    enum class Menu {
        MainMenu,
        SettingsMenu,
    };
  public:
    ScreenMainMenu(ScreenStack* stack);

    void onRender() override;

  private:
    Menu m_activeMenu = Menu::MainMenu;

    void showMainMenu();
    void showSettingsMenu();
};
