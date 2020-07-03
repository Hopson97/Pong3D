#include "Constants.h"
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Window.hpp>
#include <glad/glad.h>
#include <iostream>
#include "GL/GLDebug.h"

#include "GL/GLUtilities.h"
#include "Mesh.h"

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

    if (!gladLoadGL()) {
        std::cerr << "Failed to load OpenGL, exiting.\n";
        return 1;
    }
    initGLDebug();
    glCheck(glClearColor(0.0f, 0.0f, 0.0f, 1.0f));
    glCheck(glViewport(0, 0, window.getSize().x, window.getSize().y));
    glCheck(glEnable(GL_DEPTH_TEST));

    // clang-format off
    //Make a mesh
    Mesh mesh;
    mesh.positions = {
        0.0f, 0.0f, 0.0f,
        0.5f, 0.0f, 0.0f,
        0.5f, 0.5f, 0.0f,
        0.0f, 0.5f, 0.0f
    };

    mesh.index = {
        0, 1, 2,
        2, 3, 0
    };
    // clang-format on

    auto quad = bufferMesh(mesh);

    // Main loop
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }
        // Input

        // Update

        // Render
        glCheck(glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT));

        glCheck(glBindVertexArray(quad.vao));
        glCheck(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr));

        window.display();
    }

    return 0;
}