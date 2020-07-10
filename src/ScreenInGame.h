#pragma once

#include "Screen.h"

#include "GameObjects.h"
#include "Maths.h"
#include "GL/GLUtilities.h"

class ScreenInGame final : public Screen {
  public:
    ScreenInGame(ScreenStack* stack);
    ~ScreenInGame();

    void onInput();
    void onUpdate(float dt);
    void onRender();

  private:
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

    int m_playerScore = 0;
    int m_enemyScore = 0;

};