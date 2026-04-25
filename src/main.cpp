#include <print>

#include <SFML/Window/VideoMode.hpp>
#include <SFML/Window/Window.hpp>
#include <glad/glad.h>

#include "GUI.h"
#include "Graphics/Mesh.h"
#include "Graphics/OpenGL/Framebuffer.h"
#include "Graphics/OpenGL/GLUtils.h"
#include "Screen.h"
#include "ScreenMainMenu.h"
#include "Util/Keyboard.h"
#include "Util/Profiler.h"
#include "Util/TimeStep.h"
#include <imgui.h>

namespace
{
    void handle_event(const sf::Event& event, sf::Window& window, bool& show_debug_info,
                      bool& close_requested);
} // namespace

int main()
{
    sf::ContextSettings context_settings;
    context_settings.depthBits = 24;
    context_settings.stencilBits = 8;
    context_settings.antiAliasingLevel = 4;
    context_settings.majorVersion = 4;
    context_settings.minorVersion = 6;
    context_settings.attributeFlags = sf::ContextSettings::Debug;

    sf::Window window(sf::VideoMode::getDesktopMode(), "Pong 3D", sf::Style::None,
                      sf::State::Fullscreen, context_settings);

    window.setVerticalSyncEnabled(true);
    if (!window.setActive(true))
    {
        std::println(std::cerr, "Failed to activate the window.");
        return EXIT_FAILURE;
    }

    if (!gladLoadGL())
    {
        std::println(std::cerr, "Failed to initialise OpenGL - Is OpenGL linked correctly?");
        return EXIT_FAILURE;
    }
    gl::enable_debugging();
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glViewport(0, 0, window.getSize().x, window.getSize().y);
    gl::enable(gl::Capability::DepthTest);
    gl::enable(gl::Capability::CullFace);
    gl::cull_face(gl::Face::Back);
    glLineWidth(2.0f);

    TimeStep updater{120};
    Profiler profiler;
    bool show_debug_info = false;

    ScreenStack screens;
    screens.pushScreen(std::make_unique<ScreenMainMenu>(&screens));
    screens.update();

    if (!GUI::init(&window))
    {
        std::println(std::cerr, "Failed to initialise Imgui.");
        return EXIT_FAILURE;
    }
    auto& style = ImGui::GetStyle();
    style.WindowRounding = 2;
    style.FrameRounding = 0;
    style.PopupRounding = 0;
    style.ScrollbarRounding = 0;
    style.TabRounding = 6;

    // Frame buffers
    gl::Framebuffer framebuffer(window.getSize().x, window.getSize().y);
    framebuffer.attach_colour(gl::TextureFormat::RGBA8)
        .attach_colour(gl::TextureFormat::RGBA8)
        .attach_renderbuffer();
    if (!framebuffer.is_complete())
    {
        std::println(std::cerr, "Failed to initialise framebuffer.");
        return EXIT_FAILURE;
    }

    // MSAA
    gl::Framebuffer msaa_framebuffer(window.getSize().x, window.getSize().y);
    msaa_framebuffer.create_as_multisample_aa();
    if (!msaa_framebuffer.is_complete())
    {
        std::println(std::cerr, "Failed to initialise MSAA.");
        return EXIT_FAILURE;
    }

    // Setup blurs
    int blurRes = 4;
    auto width = window.getSize().x / blurRes;
    auto height = window.getSize().y / blurRes;

    gl::Framebuffer blurHorizontalFbo(width, height);
    blurHorizontalFbo.attach_colour(gl::TextureFormat::RGBA8).attach_renderbuffer();
    if (!blurHorizontalFbo.is_complete())
    {
        std::println(std::cerr, "Failed to initialise blurHorizontalFbo.");
        return EXIT_FAILURE;
    }

    gl::Framebuffer blurVerticalFbo(width, height);
    blurVerticalFbo.attach_colour(gl::TextureFormat::RGBA8).attach_renderbuffer();
    if (!blurVerticalFbo.is_complete())
    {
        std::println(std::cerr, "Failed to initialise blurVerticalFbo.");
        return EXIT_FAILURE;
    }

    gl::Shader blurShader;
    if (!blurShader.load_stage("assets/shaders/screen_vertex.glsl", gl::ShaderType::Vertex) ||
        !blurShader.load_stage("assets/shaders/blur_fragment.glsl", gl::ShaderType::Fragment) ||
        !blurShader.link_shaders())
    {
        std::println(std::cerr, "Failed to initialise blurShader.");
        return EXIT_FAILURE;
    }

    gl::Shader finalPassShader;
    if (!finalPassShader.load_stage("assets/shaders/screen_vertex.glsl", gl::ShaderType::Vertex) ||
        !finalPassShader.load_stage("assets/shaders/screen_fragment.glsl",
                                    gl::ShaderType::Fragment) ||
        !finalPassShader.link_shaders())
    {
        std::println(std::cerr, "Failed to initialise blurShader.");
        return EXIT_FAILURE;
    }

    finalPassShader.bind();
    finalPassShader.set_uniform("bloomTexture", 0);
    finalPassShader.set_uniform("colourTexture", 1);
    finalPassShader.set_uniform("bloomToggle", true);

    // Mesh
    gl::VertexArrayObject fboVAO;

    Keyboard keyboard;

    // -------------------
    // ==== Main Loop ====
    // -------------------

    while (window.isOpen() && screens.hasScreen())
    {
        GUI::begin_frame();
        bool close_requested = false;
        while (auto event = window.pollEvent())
        {
            GUI::event(window, *event);
            keyboard.update(*event);
            handle_event(*event, window, show_debug_info, close_requested);
        }

        auto& screen = screens.peekScreen();

        // Update
        {
            auto& update_profiler = profiler.begin_section("Update");
            screen.onInput();
            update_profiler.end_section();
        }

        // Fixed-rate update
        {
            auto& fixed_update_profiler = profiler.begin_section("Fixed Update");
            updater.update([&](sf::Time dt) { screen.onUpdate(dt.asSeconds()); });
            fixed_update_profiler.end_section();
        }

        // Render
        {
            auto& render_profiler = profiler.begin_section("Render");
            gl::enable(gl::Capability::DepthTest);
            gl::enable(gl::Capability::CullFace);

            if (Settings::get().enable_msaa)
            {
                msaa_framebuffer.bind(gl::FramebufferTarget::Framebuffer, true);
            }
            else
            {
                framebuffer.bind(gl::FramebufferTarget::Framebuffer, true);
            }
            screen.onRender();

            render_profiler.end_section();
        }

        // Begin Post Processing
        fboVAO.bind();
        gl::disable(gl::Capability::DepthTest);

        if (Settings::get().useBloomShaders)
        {
            blurShader.bind();
            blurShader.set_uniform("bloomIntenstity", Settings::get().bloom_intensity);

            // Blur the image horizontal
            blurHorizontalFbo.bind(gl::FramebufferTarget::Framebuffer, true);
            framebuffer.bind_texture(0, 0);
            blurShader.set_uniform("horizontalBlur", 1);
            glDrawArrays(GL_TRIANGLES, 0, 6);

            // Blur the image vertical
            blurVerticalFbo.bind(gl::FramebufferTarget::Framebuffer, true);
            blurHorizontalFbo.bind_texture(0, 0);
            blurShader.set_uniform("horizontalBlur", 0);
            glDrawArrays(GL_TRIANGLES, 0, 6);

            // Keep on blurring
            for (int i = 0; i < 10; i++)
            {
                blurHorizontalFbo.bind(gl::FramebufferTarget::Framebuffer, true);
                blurVerticalFbo.bind_texture(0, 0);
                blurShader.set_uniform("horizontalBlur", 1);
                glDrawArrays(GL_TRIANGLES, 0, 6);

                blurVerticalFbo.bind(gl::FramebufferTarget::Framebuffer, true);
                blurHorizontalFbo.bind_texture(0, 0);
                blurShader.set_uniform("horizontalBlur", 1);
                glDrawArrays(GL_TRIANGLES, 0, 6);
            }
        }

        // Render to the window
        if (Settings::get().enable_msaa)
        {
            msaa_framebuffer.bind(gl::FramebufferTarget::ReadFramebuffer, false);
            framebuffer.bind(gl::FramebufferTarget::DrawFramebuffer, false);
            glBlitFramebuffer(0, 0, msaa_framebuffer.width, msaa_framebuffer.height, 0, 0,
                              framebuffer.width, framebuffer.height, GL_COLOR_BUFFER_BIT,
                              GL_NEAREST);
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glViewport(0, 0, window.getSize().x, window.getSize().y);

        blurVerticalFbo.bind_texture(0, 0);
        framebuffer.bind_texture(0, 1);

        finalPassShader.bind();
        finalPassShader.set_uniform("bloomToggle", Settings::get().useBloomShaders);

        glDrawArrays(GL_TRIANGLES, 0, 6);

        // Show profiler
        profiler.end_frame();
        if (show_debug_info)
        {
            profiler.gui();
        }

        // --------------------------
        // ==== End Frame ====
        // --------------------------
        GUI::render();
        window.display();
        if (close_requested)
        {
            window.close();
        }
        screens.update();
    }

    // --------------------------
    // ==== Graceful Cleanup ====
    // --------------------------
    GUI::shutdown();
}

namespace
{
    void handle_event(const sf::Event& event, sf::Window& window, bool& show_debug_info,
                      bool& close_requested)
    {
        if (event.is<sf::Event::Closed>())
        {
            close_requested = true;
        }
        else if (auto* key = event.getIf<sf::Event::KeyPressed>())
        {

            switch (key->code)
            {
                case sf::Keyboard::Key::F1:
                    show_debug_info = !show_debug_info;
                    break;

                default:
                    break;
            }
        }
    }
} // namespace
