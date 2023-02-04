#include "Settings.h"
#include "Screen.h"
#include <fstream>
#include <imgui.h>

Settings::Settings()
{
    std::ifstream inFile("data/settings.txt");
    std::string val;
    while (inFile >> val) {
        if (val == "show_fps") {
            inFile >> showFps;
        }
        else if (val == "show_bloom") {
            inFile >> useBloomShaders;
        }
        else if (val == "render_terrain") {
            inFile >> renderTerrain;
        }
        else if (val == "sway_terrain") {
            inFile >> swayTerrain;
        }
        else if (val == "move_terrain") {
            inFile >> moveTerrain;
        }
    }
}

Settings::~Settings()
{
    std::ofstream outFile("data/settings.txt");
    outFile << "show_fps" << ' ' << showFps << '\n';
    outFile << "show_bloom" << ' ' << useBloomShaders << '\n';
    outFile << "render_terrain" << ' ' << renderTerrain << '\n';
    outFile << "sway_terrain" << ' ' << swayTerrain << '\n';
    outFile << "move_terrain" << ' ' << moveTerrain << '\n';
}

void Settings::showSettingsMenu(std::function<void(void)> onBackPressed)
{
    if (imguiBeginCustom("S E T T I N G S")) {
        ImGui::Separator();
        ImGui::Text("Basic Settings");
        ImGui::Checkbox("Show FPS Counter", &showFps);

        ImGui::Separator();
        ImGui::Text("Video Settings");
        ImGui::Checkbox("Bloom", &useBloomShaders);
        ImGui::Checkbox("Render Terrain", &renderTerrain);

        ImGui::Separator();
        ImGui::Text("Misc Settings");
        ImGui::Checkbox("Sway Terrain", &swayTerrain);
        ImGui::Checkbox("Move Terrain", &moveTerrain);

        ImGui::Separator();
        if (imguiButtonCustom("Back")) {
            onBackPressed();
        }
    }
    ImGui::End();
}
