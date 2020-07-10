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

const float ROOM_SIZE = 30.0f;
const float ROOM_DEPTH = 40.0f;

template <typename T, typename F>
void roomCollide(T& object, F onCollide)
{
    if (object.position.x + T::WIDTH > ROOM_SIZE) {
        object.position.x = ROOM_SIZE - T::WIDTH;
        onCollide(object.velocity.x);
    }
    else if (object.position.x < 0) {
        object.position.x = 0;
        onCollide(object.velocity.x);
    }
    if (object.position.y + T::HEIGHT > ROOM_SIZE) {
        object.position.y = ROOM_SIZE - T::HEIGHT;
        onCollide(object.velocity.y);
    }
    else if (object.position.y < 0) {
        object.position.y = 0;
        onCollide(object.velocity.y);
    }
}

struct Camera {
    glm::vec3 position{0.0f};
    glm::vec3 rotation{0, 180, 0};
    glm::mat4 projectionMatrix{1.0f};

    Camera(float aspectRatio, float fov)
        : projectionMatrix(createProjectionMatrix(aspectRatio, fov))
    {
    }

    glm::mat4 getProjectionView() const
    {
        return projectionMatrix * createViewMartix(position, rotation);
    }
};

struct Paddle {
    constexpr static float WIDTH = 5.0f;
    constexpr static float HEIGHT = 3.0f;
    glm::vec3 position{0.0f};
    glm::vec3 velocity{0.0f};

    void update(float dt)
    {
        position += velocity * dt;
        velocity *= 0.96;
        roomCollide(*this, [](float& v) { v = 0; });
    }
};

struct Ball {
    constexpr static float WIDTH = 0.5;
    constexpr static float HEIGHT = 0.5;

    glm::vec3 position;
    glm::vec3 velocity{0.0f};

    void update(float dt)
    {
        position += velocity * dt;
        roomCollide(*this, [](float& v) { v *= -1; });
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
    glCheck(glCullFace(GL_BACK));
    glCheck(glEnable(GL_CULL_FACE));

    // The Room
    Mesh roomMesh = createWireCubeMesh({ROOM_SIZE, ROOM_SIZE, ROOM_DEPTH});
    auto roomObject = bufferMesh(roomMesh);

    // Ball
    Mesh ballMesh = createWireCubeMesh({Ball::WIDTH, Ball::HEIGHT, Ball::WIDTH});
    auto ballObject = bufferMesh(ballMesh);

    Ball ball;
    ball.position = {ROOM_SIZE / 2.0f, ROOM_SIZE / 2.0f, ROOM_DEPTH / 2.0f};
    ball.velocity = {5, 5, 15};

    // Paddles
    Mesh paddle = createWireCubeMesh({Paddle::WIDTH, Paddle::HEIGHT, 0.5f});
    auto paddleObject = bufferMesh(paddle);

    Paddle player;
    player.position = {ROOM_SIZE / 2.0f, ROOM_SIZE / 2.0f, 0.3f};

    Paddle enemy;
    enemy.position = {ROOM_SIZE / 2.0f, ROOM_SIZE / 2.0f, ROOM_DEPTH - 1.f};

    // Camera
    Camera camera(1280.0f / 720.0f, 90);
    camera.position = {ROOM_SIZE / 2.0f, ROOM_SIZE / 2.0f, player.position.z - 3};

    // Set up the shaders
    GLuint shader = loadShaderProgram("minimal", "minimal");
    GLuint modelMatrixLocation = glCheck(glGetUniformLocation(shader, "modelMatrix"));
    GLuint pvMatrixLocation =
        glCheck(glGetUniformLocation(shader, "projectionViewMatrix"));
    glUseProgram(shader);

    // Lighting stuff
    glm::vec3 lightPosition = ball.position;
    GLuint lightPositionLocation = glCheck(glGetUniformLocation(shader, "lightPosition"));
    glCheck(glUniform3fv(lightPositionLocation, 1, glm::value_ptr(lightPosition)));

    sf::Clock timer;
    sf::Clock deltaTimer;
    auto lastMousePos = sf::Mouse::getPosition(window);
    // Main loop
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed ||
                sf::Keyboard::isKeyPressed(sf::Keyboard::Escape))
                window.close();
        }

        // Input
        float dt = deltaTimer.restart().asSeconds();
        float SPEED = 1.5f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
            player.velocity.y += SPEED;
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
            player.velocity.y -= SPEED;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
            player.velocity.x += SPEED;
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
            player.velocity.x -= SPEED;
        }

        // "AI" Input
        if (ball.velocity.z > 0) {
            if (ball.position.x > enemy.position.x) {
                enemy.velocity.x += SPEED / 4.0f;
            }
            else if (ball.position.x < enemy.position.x) {
                enemy.velocity.x += -SPEED / 4.0f;
            }
            if (ball.position.y > enemy.position.y) {
                enemy.velocity.y += SPEED / 4.0f;
            }
            else if (ball.position.y < enemy.position.y) {
                enemy.velocity.y += -SPEED / 4.0f;
            }
        }

        // Update
        player.update(dt);
        enemy.update(dt);
        ball.update(dt);

        // Test for player or opposing player scoring
        if (ball.position.z < enemy.position.z) {
            ball.velocity.z *= -1;
        }

        if (ball.position.z > player.position.z) {
            ball.velocity.z *= -1;
        }

        camera.position.x = player.position.x + Paddle::WIDTH / 2;
        camera.position.y = player.position.y + Paddle::HEIGHT / 2;

        // Render
        glCheck(glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT));
        auto projectionView = camera.getProjectionView();
        glCheck(glUniformMatrix4fv(pvMatrixLocation, 1, GL_FALSE,
                                   glm::value_ptr(projectionView)));

        lightPosition = camera.position;
        GLuint lightPositionLocation =
            glCheck(glGetUniformLocation(shader, "lightPosition"));
        glCheck(glUniform3fv(lightPositionLocation, 1, glm::value_ptr(lightPosition)));

        // Render the ball
        glCheck(glBindVertexArray(ballObject.vao));
        auto modelmatrix = createModelMatrix(ball.position, {0, 0, 0});

        glCheck(glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE,
                                   glm::value_ptr(modelmatrix)));

        glCheck(glDrawElements(GL_TRIANGLES, ballObject.indicesCount, GL_UNSIGNED_INT,
                               nullptr));

        // Render the player
        glCheck(glBindVertexArray(paddleObject.vao));
        modelmatrix = createModelMatrix(enemy.position, {0, 0, 0});
        glCheck(glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE,
                                   glm::value_ptr(modelmatrix)));
        glCheck(glDrawElements(GL_TRIANGLES, paddleObject.indicesCount, GL_UNSIGNED_INT,
                               nullptr));

        modelmatrix = createModelMatrix(player.position, {0, 0, 0});
        glCheck(glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE,
                                   glm::value_ptr(modelmatrix)));
        glCheck(glDrawElements(GL_TRIANGLES, paddleObject.indicesCount, GL_UNSIGNED_INT,
                               nullptr));

        // Render the room
        glCheck(glBindVertexArray(roomObject.vao));
        modelmatrix = createModelMatrix({0, 0, 0}, {0, 0, 0});
        glCheck(glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE,
                                   glm::value_ptr(modelmatrix)));
        glCheck(glDrawElements(GL_TRIANGLES, roomObject.indicesCount, GL_UNSIGNED_INT,
                               nullptr));
        window.display();
    }
    return 0;
}