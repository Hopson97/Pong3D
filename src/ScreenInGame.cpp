#include "ScreenInGame.h"

#include "GL/GLDebug.h"
#include "Mesh.h"
#include "Settings.h"
#include <SFML/Window/Keyboard.hpp>
#include <imgui/imgui.h>
#include <iostream>

ScreenInGame::ScreenInGame(ScreenStack* stack)
    : Screen(stack)
    , m_camera(1280.0f / 720.0f, 80)
{
    Mesh roomMesh = createWireCubeMesh({ROOM_SIZE, ROOM_SIZE, ROOM_DEPTH}, 0.3f);
    Mesh ballMesh = createWireCubeMesh({Ball::WIDTH, Ball::HEIGHT, Ball::WIDTH});
    Mesh paddle = createWireCubeMesh({Paddle::WIDTH, Paddle::HEIGHT, 0.25f});
    Mesh terrain = createTerrainMesh(0, {TERRAIN_WIDTH, TERRAIN_HEIGHT}, TILE_SIZE);

    Terrain firstTerrain;
    firstTerrain.location = {(-TERRAIN_WIDTH * TILE_SIZE) / 2, -6, 0};
    firstTerrain.vertexArray.bind();
    firstTerrain.vertexArray.addAttribute(terrain.positions, 3);
    firstTerrain.vertexArray.addAttribute(terrain.normals, 3);
    firstTerrain.vertexArray.addElements(terrain.indices);
    m_terrains.push_back(std::move(firstTerrain));
    for (int i = 0; i < 2; i++) {
        addTerrain();
    }

    m_roomVao.bind();
    m_roomVao.addAttribute(roomMesh.positions, 3);
    m_roomVao.addAttribute(roomMesh.normals, 3);
    m_roomVao.addElements(roomMesh.indices);

    m_ballVao.bind();
    m_ballVao.addAttribute(ballMesh.positions, 3);
    m_ballVao.addAttribute(ballMesh.normals, 3);
    m_ballVao.addElements(ballMesh.indices);

    m_paddleVao.bind();
    m_paddleVao.addAttribute(paddle.positions, 3);
    m_paddleVao.addAttribute(paddle.normals, 3);
    m_paddleVao.addElements(paddle.indices);

    resetGame();

    m_shader.addShader("minimal_vertex", glpp::ShaderType::Vertex);
    m_shader.addShader("minimal_fragment", glpp::ShaderType::Fragment);
    m_shader.linkShaders();
    m_shader.bind();

    m_locModelMat = m_shader.getUniformLocation("modelMatrix");
    m_locPvMat = m_shader.getUniformLocation("projectionViewMatrix");
    m_locLightPos = m_shader.getUniformLocation("lightPosition");
    m_locColour = m_shader.getUniformLocation("colour");

    // Update lighting
    glpp::loadUniform(m_locLightPos, {ROOM_SIZE / 2.0f, ROOM_SIZE, -100});
}

ScreenInGame::~ScreenInGame()
{
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
    float SPEED = 0.8f;
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
    m_camera.position.z = m_player.position.z - 4.0f;
    m_camera.rotation.y = (m_camera.position.x - ROOM_SIZE / 2) + 180;
    m_camera.rotation.x = (m_camera.position.y - ROOM_SIZE / 2) / 2;

    // Update terrain postions
    if (Settings::get().renderTerrain) {
        for (auto itr = m_terrains.begin(); itr != m_terrains.end();) {

            auto& loc = itr->location;
            if (Settings::get().swayTerrain) {
                loc.x +=
                    std::sin(m_terrainSnakeTimer.getElapsedTime().asSeconds()) * 30.0f * dt;
            }
            if (Settings::get().moveTerrain) {
                loc.z -= 45.0f * dt;
            }
            if (loc.z < -TERRAIN_LENGTH - 10.0f) {
                itr = m_terrains.erase(itr);
                addTerrain();
            }
            else {
                itr++;
            }
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
    auto projectionView = m_camera.getProjectionView();
    m_shader.bind();
    glpp::loadUniform(m_locPvMat, projectionView);

    // Render room
    glpp::loadUniform(m_locColour, {0.0, 0.65, 0.7});
    auto roomDraw = m_roomVao.getDrawable();
    roomDraw.bind();

    auto modelmatrix = createModelMatrix({0, 0, 0}, {0, 0, 0});
    glpp::loadUniform(m_locModelMat, modelmatrix);
    roomDraw.draw();
    
    // Render paddles
    auto paddleDraw = m_paddleVao.getDrawable();
    paddleDraw.bind();

    modelmatrix = createModelMatrix(m_enemy.position, {0, 0, 0});
    glpp::loadUniform(m_locModelMat, modelmatrix);
    paddleDraw.draw();

    modelmatrix = createModelMatrix(m_player.position, {0, 0, 0});
    glpp::loadUniform(m_locModelMat, modelmatrix);
    paddleDraw.draw();

    // Render balls
    auto ballDraw = m_ballVao.getDrawable();
    ballDraw.bind();

    modelmatrix = createModelMatrix(m_ball.position, m_ball.rotation);
    glpp::loadUniform(m_locModelMat, modelmatrix);
    ballDraw.draw();
    
    // Render terrain
    if (Settings::get().renderTerrain) {
        for (const auto& terrain : m_terrains) {
            glpp::Drawable terrainDrawable = terrain.vertexArray.getDrawable();
            terrainDrawable.bind();
            modelmatrix = createModelMatrix(terrain.location, {0, 0, 0});
            glpp::loadUniform(m_locModelMat, modelmatrix);

            glpp::loadUniform(m_locColour, {1.25, 0.0, 1.25});
            glCheck(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
            terrainDrawable.draw();

            glpp::loadUniform(m_locColour, {0.0, 0.0, 0.0});
            glCheck(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
            terrainDrawable.draw();
        }
    }

    if (m_isPaused) {
        if (m_showSettings) {
            Settings::get().showSettingsMenu([&] { m_showSettings = false; });
        }
        else {
            showPauseMenu();
        }
    }
}

void ScreenInGame::showPauseMenu()
{
    if (imguiBeginCustom("P A U S E D")) {
        if (imguiButtonCustom("Resume Game")) {
            m_isPaused = false;
        }
        if (imguiButtonCustom("Settings")) {
            m_showSettings = true;
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
    terrain.vertexArray.bind();
    terrain.vertexArray.addAttribute(mesh.positions, 3);
    terrain.vertexArray.addAttribute(mesh.normals, 3);
    terrain.vertexArray.addElements(mesh.indices);

    m_terrains.push_back(std::move(terrain));
}
