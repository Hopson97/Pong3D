#include "ScreenInGame.h"

#include <iostream>

#include <SFML/System/Time.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <imgui.h>

#include "Settings.h"

ScreenInGame::ScreenInGame(ScreenStack* stack)
    : Screen(stack)
    , m_camera(1280.0f / 720.0f, 80)
{

    roomMesh.buffer();
    ballMesh.buffer();
    paddleMesh.buffer();

    Terrain firstTerrain;
    firstTerrain.location = {(-TERRAIN_WIDTH * TILE_SIZE) / 2, -6, 0};
    firstTerrain.index = 0;
    firstTerrain.mesh =
        createTerrainMesh(firstTerrain.index, {TERRAIN_WIDTH, TERRAIN_HEIGHT}, TILE_SIZE);
    firstTerrain.mesh.buffer();
    m_terrains.push_back(std::move(firstTerrain));

    for (int i = 0; i < 2; i++)
    {
        addTerrain();
    }

    resetGame();

    m_shader.load_stage("assets/shaders/minimal_vertex.glsl", gl::ShaderType::Vertex);
    m_shader.load_stage("assets/shaders/minimal_fragment.glsl", gl::ShaderType::Fragment);
    m_shader.link_shaders();
    // Update lighting
    m_shader.set_uniform("lightPosition", {ROOM_SIZE / 2.0f, ROOM_SIZE, -100});
}

void ScreenInGame::onInput()
{
    if (!m_isPaused && sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape))
    {
        m_isPaused = true;
    }
    if (m_isPaused)
    {
        return;
    }
    float SPEED = 0.8f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W))
    {
        m_player.velocity.y += SPEED;
    }
    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S))
    {
        m_player.velocity.y -= SPEED;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A))
    {
        m_player.velocity.x += SPEED;
    }
    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))
    {
        m_player.velocity.x -= SPEED;
    }

    // "AI" Input
    if (m_ball.velocity.z > 0)
    {
        if (m_ball.position.x + Ball::WIDTH / 2 > m_enemy.position.x + Paddle::WIDTH / 2)
        {
            m_enemy.velocity.x += SPEED;
        }
        else if (m_ball.position.x + Ball::WIDTH / 2 < m_enemy.position.x + Paddle::WIDTH / 2)
        {
            m_enemy.velocity.x += -SPEED;
        }
        if (m_ball.position.y - Ball::HEIGHT / 2 > m_enemy.position.y + Paddle::HEIGHT / 2)
        {
            m_enemy.velocity.y += SPEED;
        }
        else if (m_ball.position.y + Ball::HEIGHT / 2 < m_enemy.position.y + Paddle::HEIGHT / 2)
        {
            m_enemy.velocity.y += -SPEED;
        }
    }
}

void ScreenInGame::onUpdate(float dt)
{
    if (m_isPaused)
    {
        return;
    }
    m_ball.update(dt);
    m_player.update(dt);
    m_enemy.update(dt);

    // Ball bouncing
    if (m_ball.aabb.isColliding(m_player.aabb))
    {
        m_ball.velocity.z = BALL_SPEED;
        m_ball.position.z = m_player.aabb.max.z + 0.1f;
        m_ball.bounceOffPaddle(m_player);
    }
    else if (m_ball.aabb.isColliding(m_enemy.aabb))
    {
        m_ball.velocity.z = -BALL_SPEED;
        m_ball.position.z = m_enemy.aabb.min.z - Ball::DEPTH;
        m_ball.bounceOffPaddle(m_enemy);
    }

    // Scoring
    if (m_ball.position.z > ROOM_DEPTH)
    {
        m_playerScore++;
        m_ball.velocity.z = -BALL_SPEED;
    }
    else if (m_ball.position.z + Ball::DEPTH < 0)
    {
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
    if (Settings::get().renderTerrain)
    {
        for (auto itr = m_terrains.begin(); itr != m_terrains.end();)
        {

            auto& loc = itr->location;
            if (Settings::get().swayTerrain)
            {
                loc.x += std::sin(m_terrainSnakeTimer.getElapsedTime().asSeconds()) * 30.0f * dt;
            }
            if (Settings::get().moveTerrain)
            {
                loc.z -= 45.0f * dt;
            }
            if (loc.z < -TERRAIN_LENGTH - 10.0f)
            {
                itr = m_terrains.erase(itr);
                addTerrain();
            }
            else
            {
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
    if (ImGui::Begin("Screen", nullptr, flags))
    {
        ImGui::Text("Player Score: %d", m_playerScore);
        ImGui::Text("Computer Score: %d", m_enemyScore);
    }
    ImGui::End();

    gl::polygon_mode(gl::Face::FrontAndBack, gl::PolygonMode::Fill);

    // Load up projection matrix stuff
    auto projectionView = m_camera.getProjectionView();
    m_shader.bind();
    m_shader.set_uniform("projectionViewMatrix", projectionView);

    // Render room
    m_shader.set_uniform("colour", {0.0, 0.65, 0.7});

    roomMesh.bind();
    roomMesh.draw_elements();

    m_shader.set_uniform("modelMatrix", createModelMatrix({0, 0, 0}, {0, 0, 0}));
    roomMesh.draw_elements();

    // Render paddles
    paddleMesh.bind();

    m_shader.set_uniform("modelMatrix", createModelMatrix(m_enemy.position, {0, 0, 0}));
    paddleMesh.draw_elements();

    m_shader.set_uniform("modelMatrix", createModelMatrix(m_player.position, {0, 0, 0}));
    paddleMesh.draw_elements();

    // Render balls
    ballMesh.bind();

    m_shader.set_uniform("modelMatrix", createModelMatrix(m_ball.position, m_ball.rotation));
    ballMesh.draw_elements();

    // Render terrain
    if (Settings::get().renderTerrain)
    {
        for (const auto& terrain : m_terrains)
        {
            terrain.mesh.bind();

            gl::polygon_mode(gl::Face::FrontAndBack, gl::PolygonMode::Line);
            m_shader.set_uniform("modelMatrix", createModelMatrix(terrain.location, {0, 0, 0}));
            m_shader.set_uniform("colour", {1.25, 0.0, 1.25});
            terrain.mesh.draw_elements();

            gl::polygon_mode(gl::Face::FrontAndBack, gl::PolygonMode::Fill);
            m_shader.set_uniform("colour", {0.0, 0.0, 0.0});
            terrain.mesh.draw_elements();
        }
    }

    if (m_isPaused)
    {
        if (m_showSettings)
        {
            Settings::get().showSettingsMenu([&] { m_showSettings = false; });
        }
        else
        {
            showPauseMenu();
        }
    }
}

void ScreenInGame::showPauseMenu()
{
    if (imguiBeginCustom("P A U S E D"))
    {
        if (imguiButtonCustom("Resume Game"))
        {
            m_isPaused = false;
        }
        if (imguiButtonCustom("Settings"))
        {
            m_showSettings = true;
        }
        if (imguiButtonCustom("Reset Game"))
        {
            resetGame();
            m_isPaused = false;
        }
        if (imguiButtonCustom("Exit Game"))
        {
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

    terrain.mesh = createTerrainMesh(terrain.index, {TERRAIN_WIDTH, TERRAIN_HEIGHT}, TILE_SIZE);
    terrain.mesh.buffer();

    m_terrains.push_back(std::move(terrain));
}
