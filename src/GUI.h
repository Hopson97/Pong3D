#pragma once

#include <SFML/Window/Window.hpp>

#include "Settings.h"
#include "Graphics/OpenGL/Texture.h"
#include "Graphics/OpenGL/Shader.h"
#include "Graphics/OpenGL/VertexArrayObject.h"


namespace GUI
{
    [[nodiscard]] bool init(sf::Window* window);

    void begin_frame();

    void shutdown();
    void render();

    void event(const sf::Window& window, sf::Event& e);
} // namespace GUI
