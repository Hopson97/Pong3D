#pragma once

#include <functional>

struct Settings
{
  private:
    Settings();
    ~Settings();

  public:
    static Settings& get()
    {
        static Settings settings;
        return settings;
    }

    // Video Settings
    bool useBloomShaders = false;
    bool renderTerrain = true;
    bool enable_msaa = false;

    float bloom_intensity = 1.0f;

    // Misc
    bool swayTerrain = true;
    bool moveTerrain = true;

    bool showFps = false;



    void showSettingsMenu(std::function<void(void)> onBackPressed);
};
