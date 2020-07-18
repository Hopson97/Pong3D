#pragma once

#include <functional>

struct Settings {
  private:
    Settings();
    ~Settings();

  public:
    static Settings& get()
    {
        static Settings settings;
        return settings;
    }

    bool isBoom = true;
    bool showFps = false;

    void showSettingsMenu(std::function<void(void)> onBackPressed);
};
