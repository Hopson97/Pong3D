#pragma once

#include "Screen.h"

class ScreenMainMenu final : public Screen {
  public:
    ScreenMainMenu(ScreenStack* stack);

    void onRender() override;

  private:
};
