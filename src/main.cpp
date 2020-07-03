#include "Constants.h"
#include "GL/GLDebug.h"
#include "GL/GLUtilities.h"
#include "Maths.h"
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Window.hpp>
#include <glad/glad.h>
#include <iostream>
#include <glm/gtc/type_ptr.hpp>

struct Camera {
    glm::vec3 position;
    glm::vec3 rotation;
    glm::mat4 projectionMatrix;

    Camera(float aspectRatio, float fov)
    :   projectionMatrix(createProjectionMatrix(aspectRatio, fov))
    {
        position = {0, 0, -1.0f};
        rotation = {0, 0, 0};
    }

    glm::mat4 getProjectionView() const
    {
        return projectionMatrix * createViewMartix(position, rotation);
    }
};

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

    //Make a mesh
    Mesh mesh;

    // clang-format off
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
    GLuint shader = loadShaderProgram("minimal", "minimal");
    GLuint modelMatrixLocation = glCheck(glGetUniformLocation(shader, "modelMatrix"));
    GLuint pvMatrixLocation = glCheck(glGetUniformLocation(shader, "projectionViewMatrix"));
    glUseProgram(shader);

    Camera camera(1280.0f / 720.0f, 90);

    // Main loop
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }
        // Input

        // Update
        auto projectionView = camera.getProjectionView();
        auto modelmatrix = createModelMatrix({0.0f, 0.5f, -3.0f}, {0.0f, 0.0f, 0.0f});

        glCheck(glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, glm::value_ptr(modelmatrix)));
        glCheck(glUniformMatrix4fv(pvMatrixLocation, 1, GL_FALSE, glm::value_ptr(projectionView)));

        // Render
        glCheck(glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT));

        glCheck(glBindVertexArray(quad.vao));
        glCheck(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr));

        window.display();
    }
    return 0;
}