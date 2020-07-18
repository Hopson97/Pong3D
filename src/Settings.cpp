#include "Settings.h"
#include "Screen.h"
#include <fstream>
#include <imgui/imgui.h>

Settings::Settings() 
{
    std::ifstream inFile("data/settings.txt");
    std::string val;
    while (inFile >> val) {
        if (val == "show_fps") {
            inFile >> showFps;
        }
        else if (val == "show_bloom") {
            inFile >> isBoom;
        }
    }
}

Settings::~Settings() 
{
    std::ofstream outFile("data/settings.txt");
    outFile << "show_fps" << ' ' << showFps << '\n';
    outFile << "show_bloom" << ' ' << isBoom << '\n';
}

void Settings::showSettingsMenu(std::function<void(void)> onBackPressed)
{
    if (imguiBeginCustom("S E T T I N G S")) {
        ImGui::Separator();
        ImGui::Text("Basic Settings");
        ImGui::Checkbox("Show FPS Counter", &showFps);

        ImGui::Separator();
        ImGui::Text("Video Settings");
        ImGui::Checkbox("Bloom", &isBoom);

        ImGui::Separator();
        if (imguiButtonCustom("Back")) {
            onBackPressed();
        }
    }
    ImGui::End();
}
