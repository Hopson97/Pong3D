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

struct Camera {
    glm::vec3 position{1, 0, 2.0f};
    glm::vec3 rotation{0, 0, 0};
    glm::mat4 projectionMatrix;

    Camera(float aspectRatio, float fov)
        : projectionMatrix(createProjectionMatrix(aspectRatio, fov))
    {
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

    // Make a mesh
    Mesh mesh = createCubeMesh({1.0f, 1.0f, 1.0f});

    auto cube = bufferMesh(mesh);
    GLuint shader = loadShaderProgram("minimal", "minimal");
    GLuint modelMatrixLocation = glCheck(glGetUniformLocation(shader, "modelMatrix"));
    GLuint pvMatrixLocation =
        glCheck(glGetUniformLocation(shader, "projectionViewMatrix"));
    glUseProgram(shader);

    Camera camera(1280.0f / 720.0f, 90);

    // Lighting stuff
    glm::vec3 lightPosition = {1.0f, 1.0f, -3.0f};
    GLuint lightPositionLocation = glCheck(glGetUniformLocation(shader, "lightPosition"));
    glCheck(glUniform3fv(lightPositionLocation, 1, glm::value_ptr(lightPosition)));

    //temp
    float dist = -2.0f;
    float rot = 0;
    sf::Clock timer;

    // Main loop
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }
        // Input
        dist = -5 + std::sin(timer.getElapsedTime().asSeconds()) * 2;
        rot += 1.0f;
        // Update
        auto projectionView = camera.getProjectionView();
        auto modelmatrix = createModelMatrix({-1.0f, -1.5f, dist}, {0, rot, 0});
        std::cout << dist << std::endl;

        glCheck(glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE,
                                   glm::value_ptr(modelmatrix)));
        glCheck(glUniformMatrix4fv(pvMatrixLocation, 1, GL_FALSE,
                                   glm::value_ptr(projectionView)));

        // Render
        glCheck(glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT));

        glCheck(glBindVertexArray(cube.vao));
        glCheck(
            glDrawElements(GL_TRIANGLES, cube.indicesCount, GL_UNSIGNED_INT, nullptr));
        glCheck(
            glDrawElements(GL_TRIANGLES, cube.indicesCount, GL_UNSIGNED_INT, nullptr));

        
        modelmatrix = createModelMatrix(lightPosition, {0, 0, 0});
        glCheck(glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE,
                                   glm::value_ptr(modelmatrix)));
        glCheck(
            glDrawElements(GL_TRIANGLES, cube.indicesCount, GL_UNSIGNED_INT, nullptr));




        window.display();
    }
    return 0;
}