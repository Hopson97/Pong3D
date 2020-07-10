#pragma once

#include "Screen.h"

class ScreenMainMenu final : public Screen {
  public:
    ScreenMainMenu(ScreenStack* stack);

    void onInput();
    void onUpdate(float dt);
    void onRender();

  private:
};
