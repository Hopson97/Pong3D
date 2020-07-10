#include "GL/GLDebug.h"
#include "GL/GLUtilities.h"
#include "Maths.h"
#include "Mesh.h"
#include <SFML/System/Time.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Window.hpp>
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

#include <imgui/imgui.h>
#include <imgui_impl/imgui_impl_opengl3.h>
#include <imgui_impl/imgui_impl_sfml.h>

#include "Screen.h"
#include "ScreenInGame.h"

int main()
{
    // Init Window, OpenGL set up etc
    sf::ContextSettings contextSettings;
    contextSettings.depthBits = 24;
    contextSettings.stencilBits = 8;
    contextSettings.antialiasingLevel = 4;
    contextSettings.majorVersion = 3;
    contextSettings.minorVersion = 3;
    contextSettings.attributeFlags = sf::ContextSettings::Core;
    sf::Window window({1280, 720}, "Pong 3D", sf::Style::Close, contextSettings);
    window.setFramerateLimit(60);

    if (!gladLoadGL()) {
        std::cerr << "Failed to load OpenGL, exiting.\n";
        return 1;
    }
    initGLDebug();
    glCheck(glClearColor(0.0f, 0.0f, 0.0f, 1.0f));
    glCheck(glViewport(0, 0, window.getSize().x, window.getSize().y));
    glCheck(glEnable(GL_DEPTH_TEST));
    glCheck(glCullFace(GL_BACK));
    glCheck(glEnable(GL_CULL_FACE));

    // ImGUI
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplSfml_Init(&window);
    ImGui_ImplOpenGL3_Init();

    ScreenStack screens;
    screens.pushScreen(std::make_unique<ScreenInGame>(&screens));
    
    // Main loop
    sf::Clock deltaTimer;
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            ImGui_ImplSfml_ProcessEvent(event);
            if (event.type == sf::Event::Closed)
                window.close();
        }
        
        Screen& screen = screens.peekScreen();
        screen.onInput();
        screen.onUpdate(deltaTimer.restart().asSeconds());

        glCheck(glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT));
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSfml_NewFrame();
        ImGui::NewFrame();

        screen.onRender();

        // Display
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        window.display();
    }
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSfml_Shutdown();
    ImGui::DestroyContext();

    return 0;
}