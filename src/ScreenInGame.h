#pragma once

#include "GameObjects.h"
#include "Graphics/OpenGL/Shader.h"
#include "Graphics/OpenGL/VertexArrayObject.h"
#include "Graphics/Mesh.h"
#include "Maths.h"
#include "Screen.h"
#include <SFML/System/Clock.hpp>

#include <list>

class ScreenInGame final : public Screen
{
    struct Terrain
    {
        int index = 0;
        glm::vec3 location{0.0f};
        Mesh3D mesh;
    };

  public:
    ScreenInGame(ScreenStack* stack, float enemySpeed);

    void onInput() override;
    void onUpdate(float dt) override;
    void onRender() override;

  private:
    bool m_isPaused = false;
    bool m_showSettings = false;
    void showPauseMenu();

    void resetGame();

    void addTerrain();

    std::list<Terrain> m_terrains;
    sf::Clock m_terrainSnakeTimer;

    Camera m_camera;
    Paddle m_player;
    Paddle m_enemy;
    Ball m_ball;

    Mesh3D roomMesh = createWireCubeMesh({ROOM_SIZE, ROOM_SIZE, ROOM_DEPTH}, 0.3f);
    Mesh3D ballMesh = createWireCubeMesh({Ball::WIDTH, Ball::HEIGHT, Ball::WIDTH});
    Mesh3D paddleMesh = createWireCubeMesh({Paddle::WIDTH, Paddle::HEIGHT, 0.25f});

    gl::Shader m_shader;

    int m_playerScore = 0;
    int m_enemyScore = 0;
    float m_enemySpeed = 0;
};