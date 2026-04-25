#include "Settings.h"
#include "Screen.h"
#include <fstream>
#include <imgui.h>

Settings::Settings()
{
    std::ifstream inFile("settings.txt");
    std::string val;
    while (inFile >> val)
    {
        if (val == "show_fps")
        {
            inFile >> showFps;
        }
        else if (val == "show_bloom")
        {
            inFile >> useBloomShaders;
        }
        else if (val == "render_terrain")
        {
            inFile >> renderTerrain;
        }
        else if (val == "enable_msaa")
        {
            inFile >> enable_msaa;
        }
        else if (val == "bloom_intensity")
        {
            inFile >> bloom_intensity;
        }

        else if (val == "sway_terrain")
        {
            inFile >> swayTerrain;
        }
        else if (val == "move_terrain")
        {
            inFile >> moveTerrain;
        }
    }
}

Settings::~Settings()
{
    std::ofstream outFile("settings.txt");
    outFile << "show_fps" << ' ' << showFps << '\n';
    outFile << "show_bloom" << ' ' << useBloomShaders << '\n';
    outFile << "render_terrain" << ' ' << renderTerrain << '\n';
    outFile << "enable_msaa" << ' ' << enable_msaa << '\n';
    outFile << "bloom_intensity" << ' ' << bloom_intensity << '\n';
    outFile << "move_terrain" << ' ' << moveTerrain << '\n';
    outFile << "sway_terrain" << ' ' << swayTerrain << '\n';
}

void Settings::showSettingsMenu(std::function<void(void)> onBackPressed)
{
    if (imguiBeginCustom("S E T T I N G S"))
    {
        ImGui::Separator();
        ImGui::Text("Basic Settings");
        ImGui::Checkbox("Show FPS Counter", &showFps);

        ImGui::Separator();
        ImGui::Text("Video Settings");
        ImGui::Checkbox("Bloom", &useBloomShaders);
        if (!useBloomShaders)
        {
            ImGui::BeginDisabled();
        }
        ImGui::SliderFloat("Bloom Intenstity", &bloom_intensity, 1.0000f, 1.225f);
        if (!useBloomShaders)
        {
            ImGui::EndDisabled();
        }
        ImGui::Checkbox("Render Terrain", &renderTerrain);
        ImGui::Checkbox("MSAA", &enable_msaa);

        ImGui::Separator();
        ImGui::Text("Misc Settings");

        if (!renderTerrain)
        {
            ImGui::BeginDisabled();
        }
        ImGui::Checkbox("Sway Terrain", &swayTerrain);
        ImGui::Checkbox("Move Terrain", &moveTerrain);
        if (!renderTerrain)
        {
            ImGui::EndDisabled();
        }
        ImGui::Separator();
        if (imguiButtonCustom("Back"))
        {
            onBackPressed();
        }
    }
    ImGui::End();
}
