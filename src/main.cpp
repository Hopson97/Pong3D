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

constexpr float ROOM_SIZE = 30.0f;
constexpr float ROOM_DEPTH = 40.0f;
constexpr float BALL_SPEED = 12.0f;

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

struct AABB {
    glm::vec3 min{0.0f};
    glm::vec3 max{0.0f};

    template <typename T>
    void update(const T& object)
    {
        min = object.position;
        max = object.position + glm::vec3{T::WIDTH, T::HEIGHT, T::DEPTH};
    }

    bool isColliding(const AABB& other) const
    {
        return (min.x <= other.max.x && max.x >= other.min.x) &&
               (min.y <= other.max.y && max.y >= other.min.y) &&
               (min.z <= other.max.z && max.z >= other.min.z);
    }
};

struct Paddle {
    constexpr static float WIDTH = 5.0f;
    constexpr static float HEIGHT = 3.0f;
    constexpr static float DEPTH = 0.5f;
    AABB aabb;
    glm::vec3 position{0.0f};
    glm::vec3 velocity{0.0f};

    void update(float dt)
    {
        position += velocity * dt;
        velocity *= 0.96;
        roomCollide(*this, [](float& v) { v = 0; });
        aabb.update(*this);
    }
};

struct Ball {
    constexpr static float WIDTH = 0.5f;
    constexpr static float DEPTH = 0.5f;
    constexpr static float HEIGHT = 0.5f;
    AABB aabb;
    glm::vec3 position{1.0f};
    glm::vec3 velocity{0.0f};

    void update(float dt)
    {
        position += velocity * dt;
        roomCollide(*this, [](float& v) { v *= -1; });
        aabb.update(*this);
    }

    void bounceOffPaddle(const Paddle& paddle)
    {
        glm::vec2 paddleCenter = {paddle.position.x + Paddle::WIDTH / 2,
                                  paddle.position.y + Paddle::HEIGHT / 2};
        glm::vec2 ballCenter = {position.x + Ball::WIDTH / 2,
                                position.y + Ball::HEIGHT / 2};
        auto offset = paddleCenter - ballCenter;
        velocity.x = -offset.x * 5;
        velocity.y = -offset.y * 5;
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

    // ImGUI
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplSfml_Init(&window);
    ImGui_ImplOpenGL3_Init();

    // The Room
    Mesh roomMesh = createWireCubeMesh({ROOM_SIZE, ROOM_SIZE, ROOM_DEPTH});
    auto roomObject = bufferMesh(roomMesh);

    // Ball
    Mesh ballMesh = createWireCubeMesh({Ball::WIDTH, Ball::HEIGHT, Ball::WIDTH});
    auto ballObject = bufferMesh(ballMesh);

    Ball ball;
    ball.position = {ROOM_SIZE / 2.0f, ROOM_SIZE / 2.0f, ROOM_DEPTH / 2.0f};
    ball.velocity = {5, 5, BALL_SPEED};

    // Paddles
    Mesh paddle = createWireCubeMesh({Paddle::WIDTH, Paddle::HEIGHT, 0.5f});
    auto paddleObject = bufferMesh(paddle);

    Paddle player;
    player.position = {ROOM_SIZE / 2.0f, ROOM_SIZE / 2.0f, 0.5f};

    Paddle enemy;
    enemy.position = {ROOM_SIZE / 2.0f, ROOM_SIZE / 2.0f, ROOM_DEPTH - 1.f};

    // Camera
    Camera camera(1280.0f / 720.0f, 90);
    camera.position = {ROOM_SIZE / 2.0f, ROOM_SIZE / 2.0f, player.position.z - 3};

    // Set up the shaders
    Shader shader = loadShaderProgram("minimal", "minimal");
    shader.use();
    GLuint modelMatrixLocation = shader.getUniformLocation("modelMatrix");
    GLuint pvMatrixLocation = shader.getUniformLocation("projectionViewMatrix");
    GLuint lightPositionLocation = shader.getUniformLocation("lightPosition");


    // Lighting stuff
    glm::vec3 lightPosition = ball.position;
    glCheck(glUniform3fv(lightPositionLocation, 1, glm::value_ptr(lightPosition)));

    int playerScore = 0;
    int enemyScore = 0;

    bool show_demo_window = true;

    sf::Clock timer;
    sf::Clock deltaTimer;
    auto lastMousePos = sf::Mouse::getPosition(window);
    // Main loop
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            ImGui_ImplSfml_ProcessEvent(event);
            if (event.type == sf::Event::Closed ||
                sf::Keyboard::isKeyPressed(sf::Keyboard::Escape))
                window.close();
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSfml_NewFrame();
        ImGui::NewFrame();

        ImGui::ShowDemoWindow(&show_demo_window);

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
            if (ball.position.x + Ball::WIDTH / 2 >
                enemy.position.x + Paddle::WIDTH / 2) {
                enemy.velocity.x += SPEED / 4.0f;
            }
            else if (ball.position.x + Ball::WIDTH / 2 <
                     enemy.position.x + Paddle::WIDTH / 2) {
                enemy.velocity.x += -SPEED / 4.0f;
            }
            if (ball.position.y - Ball::HEIGHT / 2 >
                enemy.position.y + Paddle::HEIGHT / 2) {
                enemy.velocity.y += SPEED / 4.0f;
            }
            else if (ball.position.y + Ball::HEIGHT / 2 <
                     enemy.position.y + Paddle::HEIGHT / 2) {
                enemy.velocity.y += -SPEED / 4.0f;
            }
        }

        // Update
        ball.update(dt);
        player.update(dt);
        enemy.update(dt);

        // Test for player or opposing player scoring
        if (ball.aabb.isColliding(player.aabb)) {
            ball.velocity.z = BALL_SPEED;
            ball.position.z = player.aabb.max.z + 0.1;
            ball.bounceOffPaddle(player);
        }
        else if (ball.aabb.isColliding(enemy.aabb)) {
            ball.velocity.z = -BALL_SPEED;
            ball.position.z = enemy.aabb.min.z - Ball::DEPTH;
            ball.bounceOffPaddle(enemy);
        }

        if (ball.position.z > ROOM_DEPTH) {
            playerScore++;
            ball.velocity.z = -BALL_SPEED;
        }
        else if (ball.position.z + Ball::DEPTH < 0) {
            enemyScore++;
            ball.velocity.z = BALL_SPEED;
        }

        camera.position.x = player.position.x + Paddle::WIDTH / 2;
        camera.position.y = player.position.y + Paddle::HEIGHT / 2;

        // Render prepare
        glCheck(glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT));

        auto projectionView = camera.getProjectionView();
        loadUniform(pvMatrixLocation, projectionView);

        lightPosition = camera.position;
        loadUniform(lightPositionLocation, lightPosition);

        // Render the ball
        glCheck(glBindVertexArray(ballObject.vao));

        auto modelmatrix = createModelMatrix(ball.position, {0, 0, 0});
        loadUniform(modelMatrixLocation, modelmatrix);
        ballObject.draw();

        // Render the player
        glCheck(glBindVertexArray(paddleObject.vao));

        modelmatrix = createModelMatrix(enemy.position, {0, 0, 0});
        loadUniform(modelMatrixLocation, modelmatrix);
        paddleObject.draw();

        modelmatrix = createModelMatrix(player.position, {0, 0, 0});
        loadUniform(modelMatrixLocation, modelmatrix);
        paddleObject.draw();

        // Render the room
        glCheck(glBindVertexArray(roomObject.vao));

        modelmatrix = createModelMatrix({0, 0, 0}, {0, 0, 0});
        loadUniform(modelMatrixLocation, modelmatrix);
        roomObject.draw();

        // GUI
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Display
        window.display();
    }
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSfml_Shutdown();
    ImGui::DestroyContext();

    ballObject.destroy();
    paddleObject.destroy();
    shader.destroy();

    return 0;
}