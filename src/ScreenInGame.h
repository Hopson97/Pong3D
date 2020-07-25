#pragma once

#include "GL/Shader.h"
#include "GL/VertexArray.h"
#include "GameObjects.h"
#include "Maths.h"
#include "Screen.h"
#include <SFML/System/Clock.hpp>

#include <list>

class ScreenInGame final : public Screen {
    struct Terrain {
        int index = 0;
        glm::vec3 location{0.0f};
        glpp::VertexArray vertexArray;
    };

  public:
    ScreenInGame(ScreenStack* stack);
    ~ScreenInGame();

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

    glpp::VertexArray m_paddleVao;
    glpp::VertexArray m_ballVao;
    glpp::VertexArray m_roomVao;

    glpp::Shader m_shader;
    glpp::UniformLocation m_locModelMat;
    glpp::UniformLocation m_locPvMat;
    glpp::UniformLocation m_locLightPos;
    glpp::UniformLocation m_locColour;

    int m_playerScore = 0;
    int m_enemyScore = 0;
};