#include "GL/Framebuffer.h"
#include "GL/GLDebug.h"
#include "Mesh.h"
#include "Screen.h"
#include "ScreenInGame.h"
#include "ScreenMainMenu.h"
#include "Settings.h"
#include <SFML/GpuPreference.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Window.hpp>
#include <imgui/imgui.h>
#include <imgui_impl/imgui_wrapper.h>
#include <iostream>

SFML_DEFINE_DISCRETE_GPU_PREFERENCE

void renderFpsMenu(float windowWidth)
{
    // Render GUI Stuff
    auto flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration |
                 ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings |
                 ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
    ImGui::SetNextWindowPos(ImVec2(windowWidth - 200, 10));
    if (ImGui::Begin("FPS", nullptr, flags)) {
        ImGuiIO& io = ImGui::GetIO();
        ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
    }
    ImGui::End();
}

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
    glpp::Framebuffer framebuffer(window.getSize().x, window.getSize().y);
    framebuffer.bind();
    framebuffer.attachTexture();
    framebuffer.attachTexture();
    framebuffer.finalise();

    // Blur render pass stuff
    int blurRes = 4;
    GLuint width = window.getSize().x / blurRes;
    GLuint height = window.getSize().y / blurRes;

    glpp::Framebuffer blurHorizontalFbo(width, height);
    blurHorizontalFbo.bind();
    blurHorizontalFbo.attachTexture();
    blurHorizontalFbo.finalise();

    glpp::Framebuffer blurVerticalFbo(width, height);
    blurVerticalFbo.bind();
    blurVerticalFbo.attachTexture();
    blurVerticalFbo.finalise();

    glpp::Shader blurShader;
    blurShader.addShader("screen_vertex", glpp::ShaderType::Vertex);
    blurShader.addShader("blur_fragment", glpp::ShaderType::Fragment);
    blurShader.linkShaders();
    blurShader.bind();
    glpp::UniformLocation blurLocation = blurShader.getUniformLocation("horizontalBlur");

    // Final pass
    glpp::Shader finalPassShader;
    finalPassShader.addShader("screen_vertex", glpp::ShaderType::Vertex);
    finalPassShader.addShader("screen_fragment", glpp::ShaderType::Fragment);
    finalPassShader.linkShaders();
    finalPassShader.bind();
    glpp::loadUniform(finalPassShader.getUniformLocation("bloomTexture"), 0);
    glpp::loadUniform(finalPassShader.getUniformLocation("colourTexture"), 1);
    glpp::UniformLocation bloomToggle = finalPassShader.getUniformLocation("bloomToggle");

    // Final pass render target
    auto screenMesh = createScreenMesh();
    glpp::VertexArray screenVertexArray;
    screenVertexArray.bind();
    screenVertexArray.addAttribute(screenMesh.positions, 2);
    screenVertexArray.addAttribute(screenMesh.textureCoords, 2);
    screenVertexArray.addElements(screenMesh.indices);
    glpp::Drawable screenDrawable = screenVertexArray.getDrawable();

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

        if (Settings::get().showFps) {
            renderFpsMenu((float)window.getSize().x);
        }

        Screen& screen = screens.peekScreen();
        screen.onInput();
        screen.onUpdate(deltaTimer.restart().asSeconds());

        // Render the scene to a framebuffer
        glCheck(glEnable(GL_DEPTH_TEST));
        framebuffer.bind();
        glCheck(glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT));
        screen.onRender();

        // Begin Post Processing
        screenDrawable.bind();
        glCheck(glDisable(GL_DEPTH_TEST));
        glCheck(glActiveTexture(GL_TEXTURE0));

        if (Settings::get().useBloomShaders) {
            blurShader.bind();
            // Blur the image horizontal
            blurHorizontalFbo.bind();
            glCheck(glClear(GL_COLOR_BUFFER_BIT));
            framebuffer.bindTexture(1);
            loadUniform(blurLocation, 1);
            screenDrawable.draw();

            // Blur the image vertical
            blurVerticalFbo.bind();
            glCheck(glClear(GL_COLOR_BUFFER_BIT));
            blurHorizontalFbo.bindTexture(0);
            loadUniform(blurLocation, 0);
            screenDrawable.draw();

            // Keep on blurring
            for (int i = 0; i < 10; i++) {
                blurHorizontalFbo.bind();
                glCheck(glClear(GL_COLOR_BUFFER_BIT));
                blurVerticalFbo.bindTexture(0);

                loadUniform(blurLocation, 1);
                screenDrawable.draw();

                blurVerticalFbo.bind();
                glCheck(glClear(GL_COLOR_BUFFER_BIT));
                blurHorizontalFbo.bindTexture(0);
                loadUniform(blurLocation, 0);
                screenDrawable.draw();
            }
        }

        // Render to the window
        finalPassShader.bind();
        glCheck(glBindFramebuffer(GL_FRAMEBUFFER, 0));
        glCheck(glViewport(0, 0, window.getSize().x, window.getSize().y));
        glCheck(glClear(GL_COLOR_BUFFER_BIT));

        glCheck(glActiveTexture(GL_TEXTURE0));
        blurVerticalFbo.bindTexture(0);

        glCheck(glActiveTexture(GL_TEXTURE1));
        framebuffer.bindTexture(0);

        loadUniform(bloomToggle, Settings::get().useBloomShaders);

        screenDrawable.draw();

        // Display
        ImGui_SFML_OpenGL3::endFrame();
        window.display();
        screens.update();
    }
    ImGui_SFML_OpenGL3::shutdown();

    return 0;
}