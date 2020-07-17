#include "GL/GLDebug.h"
#include "GL/GLUtilities.h"
#include "Mesh.h"
#include "Screen.h"
#include "ScreenInGame.h"
#include "ScreenMainMenu.h"
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Window.hpp>
#include <imgui/imgui.h>
#include <imgui_impl/imgui_wrapper.h>
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
    glCheck(glClearColor(0.0f, 0.0f, 0.0f, 0.0f));
    glCheck(glViewport(0, 0, window.getSize().x, window.getSize().y));
    glCheck(glEnable(GL_DEPTH_TEST));
    glCheck(glCullFace(GL_BACK));
    glCheck(glEnable(GL_CULL_FACE));
    glCheck(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
    glCheck(glLineWidth(2.0f));

    glEnable(GL_MULTISAMPLE);  

    // ImGUI
    ImGui_SFML_OpenGL3::init(window);
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

    // Blur render pass stuff
    int blurRes = 4;
    auto blurFboHori =
        makeFramebuffer(window.getSize().x / blurRes, window.getSize().y / blurRes);
    auto blurFboVert =
        makeFramebuffer(window.getSize().x / blurRes, window.getSize().y / blurRes);

    auto blurShader = loadShaderProgram("screen", "blur");
    GLuint blurLocation = blurShader.getUniformLocation("horizontalBlur");

    // Final pass
    auto screenShader = loadShaderProgram("screen", "screen");
    screenShader.use();
    loadUniform(screenShader.getUniformLocation("tex"), 0);
    loadUniform(screenShader.getUniformLocation("tex2"), 1);

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
        ImGui_SFML_OpenGL3::startFrame();

        Screen& screen = screens.peekScreen();
        screen.onInput();
        screen.onUpdate(deltaTimer.restart().asSeconds());

        // Render the scene to a framebuffer
        glCheck(glEnable(GL_DEPTH_TEST));
        framebuffer.use();
        glCheck(glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT));
        screen.onRender();

        // Begin Post Processing
        glCheck(glBindVertexArray(screenRender.vao));
        glCheck(glDisable(GL_DEPTH_TEST));
        glCheck(glActiveTexture(GL_TEXTURE0));
        blurShader.use();

        // Blur the image horizontal
        blurFboHori.use();
        glCheck(glClear(GL_COLOR_BUFFER_BIT));
        glCheck(glBindTexture(GL_TEXTURE_2D, framebuffer.textures[1]));
        loadUniform(blurLocation, 1);
        screenRender.draw();

        // Blur the image vertical
        blurFboVert.use();
        glCheck(glClear(GL_COLOR_BUFFER_BIT));
        glCheck(glBindTexture(GL_TEXTURE_2D, blurFboHori.textures[0]));
        loadUniform(blurLocation, 0);
        screenRender.draw();

        
        // Keep on blurring
        for (int i = 0; i < 10; i++) {
            blurFboHori.use();
            glCheck(glClear(GL_COLOR_BUFFER_BIT));
            glCheck(glBindTexture(GL_TEXTURE_2D, blurFboVert.textures[0]));
            loadUniform(blurLocation, 1);
            screenRender.draw();

            blurFboVert.use();
            glCheck(glClear(GL_COLOR_BUFFER_BIT));
            glCheck(glBindTexture(GL_TEXTURE_2D, blurFboHori.textures[0]));
            loadUniform(blurLocation, 0);
            screenRender.draw();
        }
        


        // Render to the window
        screenShader.use();
        glCheck(glBindFramebuffer(GL_FRAMEBUFFER, 0));
        glCheck(glViewport(0, 0, window.getSize().x, window.getSize().y));
        glCheck(glClear(GL_COLOR_BUFFER_BIT));

        glCheck(glActiveTexture(GL_TEXTURE0));
        glCheck(glBindTexture(GL_TEXTURE_2D, blurFboVert.textures[0]));
        glCheck(glActiveTexture(GL_TEXTURE1));
        glCheck(glBindTexture(GL_TEXTURE_2D, framebuffer.textures[0]));

        screenRender.draw();

        // Display
        ImGui_SFML_OpenGL3::endFrame();
        window.display();
        screens.update();
    }
    ImGui_SFML_OpenGL3::shutdown();

    framebuffer.destroy();
    blurFboHori.destroy();
    blurFboVert.destroy();

    blurShader.destroy();
    screenRender.destroy();
    screenShader.destroy();
    return 0;
}