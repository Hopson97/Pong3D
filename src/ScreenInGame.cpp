#include "ScreenInGame.h"

#include "GL/GLDebug.h"
#include "Mesh.h"
#include <SFML/Window/Keyboard.hpp>
#include <imgui/imgui.h>
#include <iostream>

ScreenInGame::ScreenInGame(ScreenStack* stack)
    : Screen(stack)
    , m_camera(1280.0f / 720.0f, 80)
{
    Mesh roomMesh = createWireCubeMesh({ROOM_SIZE, ROOM_SIZE, ROOM_DEPTH});
    Mesh ballMesh = createWireCubeMesh({Ball::WIDTH, Ball::HEIGHT, Ball::WIDTH});
    Mesh paddle = createWireCubeMesh({Paddle::WIDTH, Paddle::HEIGHT, 0.5f});
    Mesh terrain = createTerrainMesh(0, {TERRAIN_WIDTH, TERRAIN_HEIGHT}, TILE_SIZE);

    Terrain firstTerrain;
    firstTerrain.location = {(-TERRAIN_WIDTH * TILE_SIZE) / 2, -6, 0};
    firstTerrain.vao = bufferMesh(terrain);
    m_terrains.push_back(firstTerrain);

    for (int i = 0; i < 2; i++) {
        addTerrain();
    }

    m_roomObj = bufferMesh(roomMesh);
    m_ballObj = bufferMesh(ballMesh);
    m_paddleObj = bufferMesh(paddle);

    resetGame();

    m_shader = loadShaderProgram("minimal", "minimal");
    m_shader.use();

    m_modelMatLoc = m_shader.getUniformLocation("modelMatrix");
    m_pvMatLoc = m_shader.getUniformLocation("projectionViewMatrix");
    m_lightLoc = m_shader.getUniformLocation("lightPosition");
    m_colourLoc = m_shader.getUniformLocation("colour");
}

ScreenInGame::~ScreenInGame()
{
    m_ballObj.destroy();
    m_shader.destroy();
    m_paddleObj.destroy();
    m_roomObj.destroy();

    for (auto& terrain : m_terrains) {
        terrain.vao.destroy();
    }

    glCheck(glUseProgram(0));
    glCheck(glBindVertexArray(0));
}

void ScreenInGame::onInput()
{
    if (!m_isPaused && sf::Keyboard::isKeyPressed(sf::Keyboard::Escape)) {
        m_isPaused = true;
    }
    if (m_isPaused) {
        return;
    }
    float SPEED = 1.5f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
        m_player.velocity.y += SPEED;
    }
    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
        m_player.velocity.y -= SPEED;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
        m_player.velocity.x += SPEED;
    }
    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
        m_player.velocity.x -= SPEED;
    }

    // "AI" Input
    if (m_ball.velocity.z > 0) {
        if (m_ball.position.x + Ball::WIDTH / 2 >
            m_enemy.position.x + Paddle::WIDTH / 2) {
            m_enemy.velocity.x += SPEED / 4.0f;
        }
        else if (m_ball.position.x + Ball::WIDTH / 2 <
                 m_enemy.position.x + Paddle::WIDTH / 2) {
            m_enemy.velocity.x += -SPEED / 4.0f;
        }
        if (m_ball.position.y - Ball::HEIGHT / 2 >
            m_enemy.position.y + Paddle::HEIGHT / 2) {
            m_enemy.velocity.y += SPEED / 4.0f;
        }
        else if (m_ball.position.y + Ball::HEIGHT / 2 <
                 m_enemy.position.y + Paddle::HEIGHT / 2) {
            m_enemy.velocity.y += -SPEED / 4.0f;
        }
    }
}

void ScreenInGame::onUpdate(float dt)
{
    if (m_isPaused) {
        return;
    }
    m_ball.update(dt);
    m_player.update(dt);
    m_enemy.update(dt);

    // Ball bouncing
    if (m_ball.aabb.isColliding(m_player.aabb)) {
        m_ball.velocity.z = BALL_SPEED;
        m_ball.position.z = m_player.aabb.max.z + 0.1f;
        m_ball.bounceOffPaddle(m_player);
    }
    else if (m_ball.aabb.isColliding(m_enemy.aabb)) {
        m_ball.velocity.z = -BALL_SPEED;
        m_ball.position.z = m_enemy.aabb.min.z - Ball::DEPTH;
        m_ball.bounceOffPaddle(m_enemy);
    }

    // Scoring
    if (m_ball.position.z > ROOM_DEPTH) {
        m_playerScore++;
        m_ball.velocity.z = -BALL_SPEED;
    }
    else if (m_ball.position.z + Ball::DEPTH < 0) {
        m_enemyScore++;
        m_ball.velocity.z = BALL_SPEED;
    }

    // Update camera
    m_camera.position.x = m_player.position.x + Paddle::WIDTH / 2;
    m_camera.position.y = m_player.position.y + Paddle::HEIGHT / 2;
    m_camera.position.z = m_player.position.z - 3.0f;
    // m_camera.rotation.y = m_camera.position.x - ROOM_SIZE / 2 + 180;
    // m_camera.rotation.x = m_camera.position.y - ROOM_SIZE / 2;

    // Update terrain postions
    for (auto itr = m_terrains.begin(); itr != m_terrains.end();) {
        auto& loc = itr->location;
        loc.z -= 25.0f * dt;
        loc.x += std::sin(m_terrainSnakeTimer.getElapsedTime().asSeconds()) / 8.0f;
        if (loc.z < -TERRAIN_LENGTH - 10.0f) {
            itr->vao.destroy();
            itr = m_terrains.erase(itr);
            addTerrain();
        }
        else {
            itr++;
        }
    }
}

void ScreenInGame::onRender()
{
    // Render GUI Stuff
    auto flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration |
                 ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings |
                 ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
    ImGui::SetNextWindowPos(ImVec2(10, 10));
    if (ImGui::Begin("Screen", nullptr, flags)) {
        ImGui::Text("Player Score: %d", m_playerScore);
        ImGui::Text("Computer Score: %d", m_enemyScore);
    }
    ImGui::End();

    // Load up projection matrix stuff
    m_shader.use();
    auto projectionView = m_camera.getProjectionView();
    loadUniform(m_pvMatLoc, projectionView);

    // Update lighting
    loadUniform(m_lightLoc, m_camera.position);

    // Render room
    loadUniform(m_colourLoc, {0.0, 0.65, 0.7});
    glCheck(glBindVertexArray(m_roomObj.vao));

    auto modelmatrix = createModelMatrix({0, 0, 0}, {0, 0, 0});
    loadUniform(m_modelMatLoc, modelmatrix);
    m_roomObj.draw();

    // Render paddles
    glCheck(glBindVertexArray(m_paddleObj.vao));

    modelmatrix = createModelMatrix(m_enemy.position, {0, 0, 0});
    loadUniform(m_modelMatLoc, modelmatrix);
    m_paddleObj.draw();

    modelmatrix = createModelMatrix(m_player.position, {0, 0, 0});
    loadUniform(m_modelMatLoc, modelmatrix);
    m_paddleObj.draw();

    // Render balls
    glCheck(glBindVertexArray(m_ballObj.vao));

    modelmatrix = createModelMatrix(m_ball.position, m_ball.rotation);
    loadUniform(m_modelMatLoc, modelmatrix);
    m_ballObj.draw();

    // Render terrain
    for (const auto& terrain : m_terrains) {
        glCheck(glBindVertexArray(terrain.vao.vao));
        modelmatrix = createModelMatrix(terrain.location, {0, 0, 0});
        loadUniform(m_modelMatLoc, modelmatrix);

        loadUniform(m_colourLoc, {1.0, 0.0, 1.0});
        glCheck(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
        terrain.vao.draw();

        loadUniform(m_colourLoc, {0.0, 0.0, 0.0});
        glCheck(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
        terrain.vao.draw();
    }

    if (m_isPaused) {
        showPauseMenu();
    }
}

void ScreenInGame::showPauseMenu()
{
    if (imguiBeginCustom("P A U S E D")) {
        if (imguiButtonCustom("Resume Game")) {
            m_isPaused = false;
        }
        if (imguiButtonCustom("Reset Game")) {
            resetGame();
            m_isPaused = false;
        }
        if (imguiButtonCustom("Exit Game")) {
            m_pScreens->popScreen();
        }
    }
    ImGui::End();
}

void ScreenInGame::resetGame()
{
    m_playerScore = 0;
    m_enemyScore = 0;

    m_player.position = {ROOM_SIZE / 2.0f, ROOM_SIZE / 2.0f, 0.5f};
    m_enemy.position = {ROOM_SIZE / 2.0f, ROOM_SIZE / 2.0f, ROOM_DEPTH - 1.f};
    m_ball.position = {ROOM_SIZE / 2.0f, ROOM_SIZE / 2.0f, ROOM_DEPTH / 2.0f};

    m_ball.velocity = {5, 5, BALL_SPEED};
    m_player.velocity = {0.0f, 0.0f, 0.0f};
    m_enemy.velocity = {0.0f, 0.0f, 0.0f};
}

void ScreenInGame::addTerrain()
{
    Terrain terrain;
    terrain.location = m_terrains.back().location;
    terrain.location.z += TERRAIN_LENGTH - TILE_SIZE;
    terrain.index = m_terrains.back().index + 1;

    auto mesh =
        createTerrainMesh(terrain.index, {TERRAIN_WIDTH, TERRAIN_HEIGHT}, TILE_SIZE);
    terrain.vao = bufferMesh(mesh);

    m_terrains.push_back(terrain);
}
