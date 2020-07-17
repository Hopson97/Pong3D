#pragma once

#include "Maths.h"

constexpr float ROOM_SIZE = 30.0f;
constexpr float ROOM_DEPTH = 50.0f;
constexpr float BALL_SPEED = 15.0f;

constexpr float TERRAIN_HEIGHT = 25.0f;
constexpr float TERRAIN_WIDTH = 70.0f;
constexpr float TILE_SIZE = 30.0f;
constexpr float TERRAIN_LENGTH = TERRAIN_HEIGHT * TILE_SIZE;

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

    Camera(float aspectRatio, float fov);
    glm::mat4 getProjectionView() const;
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

    void update(float dt);
};

struct Ball {
    constexpr static float WIDTH = 0.5f;
    constexpr static float DEPTH = 0.5f;
    constexpr static float HEIGHT = 0.5f;
    AABB aabb;
    glm::vec3 position{0.0f};
    glm::vec3 rotation{0.0f};
    glm::vec3 velocity{0.0f};

    void update(float dt);
    void bounceOffPaddle(const Paddle& paddle);
};