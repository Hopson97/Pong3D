#include "GL/GLDebug.h"
#include "GL/GLUtilities.h"
#include "Mesh.h"
#include "Screen.h"
#include "ScreenInGame.h"
#include "ScreenMainMenu.h"
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Window.hpp>
#include <imgui/imgui.h>
#include <imgui_impl/imgui_impl_opengl3.h>
#include <imgui_impl/imgui_impl_sfml.h>
#include <iostream>

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
    glCheck(glLineWidth(3.0f));

    // ImGUI
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplSfml_Init(&window);
    ImGui_ImplOpenGL3_Init();
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 2;
    style.FrameRounding = 0;
    style.PopupRounding = 0;
    style.ScrollbarRounding = 0;
    style.TabRounding = 6;

    ScreenStack screens;
    screens.pushScreen(std::make_unique<ScreenMainMenu>(&screens));
    screens.update();

    // Framebuffer stuff
    auto framebuffer = makeFramebuffer(window.getSize().x, window.getSize().y);
    auto screenShader = loadShaderProgram("screen", "screen");
    auto screenRender = bufferScreenMesh(createScreenMesh());
    
    // Main loop
    sf::Clock deltaTimer;
    while (window.isOpen() && screens.hasScreen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            ImGui_ImplSfml_ProcessEvent(event);
            if (event.type == sf::Event::Closed)
                window.close();
        }

        Screen& screen = screens.peekScreen();
        screen.onInput();
        screen.onUpdate(deltaTimer.restart().asSeconds());

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSfml_NewFrame();
        ImGui::NewFrame();

        // Render to the framebuffer
        framebuffer.use();
        glCheck(glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT));
        screen.onRender();

        // Render to the window
        glCheck(glBindFramebuffer(GL_FRAMEBUFFER, 0));
        glCheck(glViewport(0, 0, window.getSize().x, window.getSize().y));
        glCheck(glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT));

        glCheck(glBindVertexArray(screenRender.vao));
        screenShader.use();

        glCheck(glBindTexture(GL_TEXTURE_2D, framebuffer.texture));
        screenRender.draw();

        // Display
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        window.display();
        screens.update();
    }
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSfml_Shutdown();
    ImGui::DestroyContext();

    // framebuffer.destroy();

    return 0;
}