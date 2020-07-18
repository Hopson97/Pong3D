#pragma once

#include "Screen.h"

#include "GL/GLUtilities.h"
#include "GameObjects.h"
#include "Maths.h"
#include <SFML/System/Clock.hpp>

#include <list>

class ScreenInGame final : public Screen {
    struct Terrain {
        int index = 0;
        glm::vec3 location{0.0f};
        BufferedMesh vao;
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

    BufferedMesh m_paddleObj;
    BufferedMesh m_ballObj;
    BufferedMesh m_roomObj;

    Shader m_shader;
    GLuint m_modelMatLoc = 0;
    GLuint m_pvMatLoc = 0;
    GLuint m_lightLoc = 0;
    GLuint m_colourLoc = 0;

    int m_playerScore = 0;
    int m_enemyScore = 0;
};